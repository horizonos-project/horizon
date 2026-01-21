#!/bin/bash

mkdir -pv init/{bin,etc}

echo "Message of the Day: This commit is probably broken!.\n" > init/etc/motd
echo "Welcome to HorizonOS!" > init/etc/welc

