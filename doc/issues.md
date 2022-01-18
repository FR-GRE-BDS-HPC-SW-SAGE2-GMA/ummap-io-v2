Issues
======

This page recap the issues encoutered with the developpement and usage of this library.

Mixing ummap-io policies with read()/write() operations
-------------------------------------------------------

Ummap-io permits to control the memory consumption of a memory segment. It means that 
part of the segment can be unmapped and marked as non accessible (`PROT_NONE`).
In this case, if we want to call the system read() and write() operations
the operating system will try to apply the operation on the begenning of the segment.
If it is mapped it will write only the part which is mapped and stopped when it
encounter a non-mapped page.

If the first page of the segment is not mapped the read() and write() operations will
simply fail.

A workaround has been propose with the `libummap-io-rw-wrapper.so` library to be
preloading in the application to wrap the read() and write() system calls making
the necessary to preload a part of the segment before entering in the system call.

Notice that the wrapper will still partially mapped the segment which make the
read() and write() function retuning a value such that just a part of the segment
has been used and you need to make a new call to progress. This is due to the fact
that we still keep only the maximum memory consumption allowed by the policy if
there is one.
