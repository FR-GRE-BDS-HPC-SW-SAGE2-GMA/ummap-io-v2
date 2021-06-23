ummap-io-v2
===========

This is a C++ re-implementation of the original ummap-io (https://github.com/sergiorg-kth/ummap-io) 
project by Sergio Rivas-Gomez from KTH for the SAGE2 european project.
The goal of this library is the handling of a file memory mapping totaly controled in userspace.

Opposite to UMAP (https://github.com/LLNL/umap) from LNLL it handle the mapping via the sefault 
ignal handling so do not require a specific kernel version. This better portability comes with
some limitation notably on performance opposite to the userfauld apporach.


What adds this V2
-----------------

Compared to the original prototype, this second version was mostly done to explore :

 - Multi-threading support.
 - Multiple driver support.
 - The URI system to build the drivers and policies from strings.
 - Mapping a storage segment not aligned to the page size (offset & size).
 - Finer grain eviction policy handling inside the process instead of a global multi-process handling.
 - More verbose error messages and strict health checking.
 - Unit tests.

Dependencies
------------

To build you need:
 - A C++11 compatible compiler
 - cmake > 2.8

For usage there is currently no dependencies.

Build
-----

This project use cmake to build. It also provide a wrapper configure script
to ease usage, just use it as autocools projects.

```sh
mkdir build
cd build
../configure
make
```

Then you can run the tests:

```sh
make test
```

Then to install

```sh
make install
```

To get more options you can look on:

```sh
../configure --help
```

It offers
---------

Ummap-io offset several drivers:

 - Dummy to trash the data.
 - Posix file driver to read/write from files.
 - Support for the Motr API (the seagate object storage).
 - Support the iocatcher RDMA server from SAGE2 project.

It also offsers several policies:

  - FIFO
  - LIFO
  - FIFO+Window

You can also share policies between mappings using the policy groups or the 
quota mecanism.

Basic usage
-----------

The minimum usage of ummap-io correspond to:

```c
//include
#include <ummap.h>

//init the library
ummap_init();

//create the driver to be used
ummap_driver_t * driver = ummap_driver_create_uri("file://./test.raw");

//establish the mapping
char * ptr = (char*)ummap(NULL, size, segment_size, 0, PROT_READ|PROT_WRITE, 0, driver, NULL, "none");

//write data
memset(ptr, 10, size);

//sync to the storage
umsync(ptr, 0, 0);

//unmap
umunmap(ptr, 0);

//fini the library
ummap_finalize();
```

More advanced examples
----------------------

You can find more advanced examples in the `./examples` directory.

License
-------

This project is distributed under Apache 2.0 license.

Cite
----

You can cite the original paper made for the first implementation:

```
"uMMAP-IO: User-level Memory-mapped I/O for HPC"
S. Rivas-Gomez, A. Fanfarillo, S. Valat, C. Laferriere, P. Couvee, S. Narasimhamurthy, and S. Markidis.
26th IEEE International Conference on High-Performance Computing, Data, and Analytics (HiPC 2019)
```
