** libk - libkernel **

This is a freestanding library that the kernel can build on in a completely
freestanding environment.

The library contains incredibly basic functions to be used in the kernel iself
and only the kernel itself. Libk is built separate and then linked into the
kernel itself so you can inspect it on its own.
