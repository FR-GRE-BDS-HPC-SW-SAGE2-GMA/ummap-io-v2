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

/*
 * This example evaluates the capability of the driver to handle a large IO (>100MB) with MERO
 * We initialize the MERO object with 1GB of data which should lead to 20 ops sent to MERO
 * The functions from the Clovis driver are directly called, there is no use to ummap in this example
 * 
 * Note that you will need a MERO instance running, see `sudo m0singlenode status`  
 * You can pass a MERO object ID as parameter or it will use on a default object ID
 * You will need the ressource file in the same directory as the exec 
 */

int main(int argc, char **argv)
{
    struct m0_uint128 id;

    char *pwd = dirname(argv[0]);
    char ressource_file[180];
    sprintf(ressource_file, "%s/mero_ressource_file.rc" , pwd);
    printf("MERO Ressource is %s\n", ressource_file);

    if (argc == 3) {
        id.u_hi = atoi(argv[1]);
        id.u_lo = atoi(argv[2]);
        printf("Reading Object ID from input parameter ID=%lu:%lu\n", id.u_hi, id.u_lo);
    } else {
        id.u_hi = 123;
        id.u_lo = 456;
        printf("Using Default Object ID=%lu:%lu\n", id.u_hi, id.u_lo);
    }

    int ret = c0appz_init(0, ressource_file);
    if (ret == 0) {
       printf("[Success] Mero Initialized\n");
    } else {
        printf("[Failed] Mero not Initialized\n");
    }

    ret = create_object(id);
    // Call ummap
    size_t size = 1000*1024*1024; //1GB
    size_t segsize = 100*1024*1024; //16MB
    off_t offset = 0;
    void *baseptr = NULL;

    ummap_init();
    baseptr = ummap(NULL, size, segsize, offset, PROT_READ|PROT_WRITE, UMMAP_NO_FIRST_READ, ummap_driver_create_clovis(id.u_hi, id.u_lo, false) , NULL, "none");
    char *index = (char *)baseptr;

    // Write the whole object with 1GB of data in it
    int i;
    for(i = 0 ; i < 1000*1024*1024; i++) {
        index[i] = 'A';
    }
    umsync(index, size, 0);
    umunmap(index, 0);

    ummap_finalize();

    return 0;
}
