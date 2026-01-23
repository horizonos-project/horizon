#!/bin/bash

mkdir -pv init/{sbin,bin,etc}

echo "Message of the Day: This commit is probably broken!.\n" > init/etc/motd
echo "Welcome to HorizonOS!" > init/etc/welc

# Remove existing sbin/init and recompile init/init.c in-place
# move new init binary to sbin directory
# via Docker, write all contents of ./init/ dir into an ext2 image
# move entire image (*.img) to build dir
#