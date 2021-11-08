Concepts
========

This page provide some defintions of concepts used in the ummap-io-v2 code.

Mapping
-------

A memory mapping is an area in the virtual addressed space when is mapped the given object.
This correspond in the Unix semantic to an mmap memory space.

Segment
-------

A mapping is splitted into segments which are the equivalent of the pages in the mmap semantic.
The difference with segments is such it can be composed of multiple pages. Its size should
be a multiple of the page size.

The segments represent the minimal granulatiry to track the dirty/mapped status. It has three
states in the ummap-io-v2 semantic:
 
 - mapped: tracking the mapping status of the segments.
 - dirty: if the content of the segment has been written and need to be flushed to the storage.
 - need read: if the segment need to be loaded from storage when passing from not mapped to mapped.
 It's an optimization when we map a new file not having to read the zeroes from the storage.

Driver
------

Ummap-io-v2 can use several IO backends which are implemented on the form of drivers. A driver
keep track of the file/object/memory to use as storage element and provide the read/write/sync
operations.

As an advanced features a driver can also provide direct memory mapping operations to get
full access to NVDIMM just following the same idea of the FSDAX approach at the OS level.

Policy
------

We can attach a policy to the mapping permitting to control the maximum amount of real
physical memory it requires. The policy track the memory consumption and provide
eviction rules to keep the memory at its limit.

The mapping can have two policies:
 - A local which manage only the local memory mapping.
 - A global which can track the memory of a group of memory mappings.

Quota
-----

A quota is a way to control the memory dispatch in a balanced way between several policies.
There is two implementations:

 - The local quota balances the memory over the registered policies.
 - The inter process quota implementation control the memory used by several processes
   communicating via a shared memory segment and signals. The group of processes
   is identified by a group name.

Mapping registry
----------------

Track all the mapping created and handled by ummap-io-v2. This is required to find the related
mapping when getting a SEGFAULT on a related address.

Global handler
--------------

This contain all the necessary to make ummap-io-v2 working. It aggregated in a single class
all what needs to be setted up as a global variables to make ummap-io-v2 working.

Policy registry
---------------

Track the policy groups so we can use the group name when we want to attatch a segment to the
given global policy.

URI: Unified Ressource Identifier
---------------------------------

This identifier permits to init a driver in the caller application without detailing
in the code which driver we are using. This permits to support any driver in an app
without implementation the exact building of each driver. 

This can be seen as an extra feature of ummap-io-v2 which could have been done
on top of it and outside.

Ressource
---------

Drivers like the Mero/Clovis on requiers the initialization and de-init of the related
framework. In order to keep abstracton via the URI system the RessourceHandler provide
the necessary tracking to automatically init and destroy extra ressources like 
Clovis/Mero.

The parameteres of the related ressources can be setup event if the driver is not available.
So we can build code independent of the avilability of optional drivers.

Listings
--------

Drivers like the Clovis/Mero one requier identifying the objects with an ID. If we do not
want to hard code IDs inside the application we can use the listings to generate and keep
track of the selected IDs by maching them with a name in the listing file.

**CAUTION**: There is currently a limitation, a listing file shoulnd not be used by
multiple processes except if it already contain all the necessary object IDs and do
not need to generate new ones.

URI variables
-------------

When using an MPI application we might want to get an ouptut file name which contain
the rank. If we want to provide the output in a config file to be able to change
the IO driver in use easily we might want to be able to add the rank dynamically
to the filename we are using. This is the reason of the URI variables which can be
registered by the application and replaced in the uris.

For example: ```file://output-{rank}.raw```.
