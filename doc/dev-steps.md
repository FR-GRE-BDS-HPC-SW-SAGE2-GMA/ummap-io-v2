Developpement steps
===================

The `/dev` directory provide several scripts to help developpement of the tool, this page
provide help to use them.

gen-coverage
------------

This script is a helper to generate coverage html report after running the tests.

Usage:

```sh
mkdir build
cd build
../configure --enable-debug --enable-coverage
make -j8
make test
../dev/gen-coverage.sh
```

Generating doxgen
-----------------

The project provide a `Doxyfile` to generate documentation with doxygen.

Usage (from the root of project):

```sh
doxygen
```
