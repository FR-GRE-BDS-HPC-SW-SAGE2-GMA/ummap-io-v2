#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libgen.h> //For dirname

#include <clovis_api.h>
#include <ummap.h>

#define TRUE 1
#define FALSE 0

#define MERO_BLOCK_SIZE 1024*1024 //1MB

/*
 * This example is a basic example on how to use the MERO driver abstraction developped for ummap-io 
 * It initializes an object, then map it, add some data and umap it, we read it at the end to see if the changes are here
 * Note that you will need a MERO instance running See `sudo m0singlenode status`  
 * 
 * You can pass a MERO object ID as parameter or it will use on a default object ID
 */
int main(int argc, char **argv)
{
    struct m0_uint128 id;
 
    if (argc == 3) {
        id.u_hi = atoi(argv[1]);
        id.u_lo = atoi(argv[2]);
        printf("Reading Object ID from input parameter ID=%lu:%lu\n", id.u_hi, id.u_lo);
    } else {
        id.u_hi = 123;
        id.u_lo = 456;
        printf("Using Default Object ID=%lu:%lu\n", id.u_hi, id.u_lo);
    }

    struct m0_indexvec ext;
    struct m0_bufvec data;
    struct m0_bufvec attr;
    int last_index = 0, i = 0;

    char *pwd = dirname(argv[0]);
    char ressource_file[180];
    sprintf(ressource_file, "%s/mero_ressource_file.rc" , pwd);
    printf("MERO Ressource is %s\n", ressource_file);

    int ret = c0appz_init(0 , ressource_file);
    if (ret == 0) {
        printf("[Success] Mero Initialized\n");
    } else {
        printf("[Failed] Mero not Initialized\n");
    }

    ret = create_object(id);
    if (ret == 0) {
        printf("[Success] Mero Object ID=%lu:%lu created\n", id.u_hi, id.u_lo);
    } else if (ret == 1) {
        printf("[Success] Mero Object ID=%lu:%lu already exists, nothing done\n", id.u_hi, id.u_lo);
    } else {
        printf("[Failed] Mero Object ID=%lu:%lu not created\n", id.u_hi, id.u_lo);        
    }


    // Init the object with 10MB of some random data in it
    /*
     * There is a limit of blocksize and count fixed in MERO
     * See CLOVIS_MAX_DATA_UNIT_PER_OPS explanation in ClovisDriver.hpp 
     */
    int blocksize = MERO_BLOCK_SIZE;
    int count = 10;

    char random_data[MERO_BLOCK_SIZE];
    sprintf(random_data, "GREG IS CRYING IN BLOOD AND TEARS HERE\n");

    m0_bufvec_alloc(&data, count, blocksize);
    m0_bufvec_alloc(&attr, count, 1);
    m0_indexvec_alloc(&ext, count);

    for (i = 0; i < count; i++) {
        /* Extent Array*/ 
        ext.iv_index[i] = last_index;
        ext.iv_vec.v_count[i] = blocksize;
        last_index += blocksize;

        /* Attribute Array*/
        attr.ov_vec.v_count[i] = 1;

        /* Data Array*/
        memcpy(data.ov_buf[i], random_data, MERO_BLOCK_SIZE);
    }

    ret = write_data_to_object(id, &ext, &data, &attr);
    if (ret == 0) {
        printf("[Success] Init Mero Object ID=%lu:%lu with %dB of some random data\n", id.u_hi, id.u_lo, blocksize * count);
    } else {
        printf("[Failed] Cannot write to Mero Object ID=%lu:%lu\n", id.u_hi, id.u_lo);
    }


    ret = read_data_from_object(id, &ext, &data, &attr);
    if (ret == 0) {
        printf("[Success] Reading %d blocks from Mero Object ID=%lu:%lu\n", data.ov_vec.v_nr, id.u_hi, id.u_lo);
    } else {
        printf("[Failed] Cannot read the Mero Object ID=%lu:%lu\n", id.u_hi, id.u_lo);
    }

    char result[MERO_BLOCK_SIZE];
    memcpy(result , data.ov_buf[0], data.ov_vec.v_count[0]);
    printf("First block of MERO object contains data: %s", result);

    // Call ummap
    size_t size = 10*MERO_BLOCK_SIZE;
    size_t segsize = MERO_BLOCK_SIZE;
    off_t offset = 0;
    void *baseptr = NULL;

    ummap_init();
    baseptr = ummap(size, segsize, offset, UMMAP_PROT_RW, ummap_driver_create_clovis(id) , NULL, "none");

    // Set some random value on the allocation
    char *index = (char *)baseptr;
    sprintf(index, "GREG IS A BIT MORE HAPPY WITH UMMAP OBJECTS LIKE THIS\n");

    umsync(baseptr, size, 0);
    umunmap(baseptr);


    // Check results 
    ret = read_data_from_object(id, &ext, &data, &attr);
    if (ret == 0) {
        printf("[Success] Reading %d blocks from Mero Object ID=%lu:%lu\n", data.ov_vec.v_nr, id.u_hi, id.u_lo);
    } else {
        printf("[Failed] Cannot read the Mero Object ID=%lu:%lu\n", id.u_hi, id.u_lo);
    }

    memcpy(result , data.ov_buf[0], data.ov_vec.v_count[0]);
    printf("First block of MERO object contains data: %s", result);

    ummap_finalize();
    c0appz_free();

    return 0;
}