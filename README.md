ummap-io-v2
===========

This is a C++ re-implementation of the original ummap-io (https://github.com/sergiorg-kth/ummap-io) 
project from Sergio Rivas-Gomez. 
The goal is to handle a file memory mapping totaly controled in userspace.

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

License
-------

This project is distributed under Apache 2.0 license.
