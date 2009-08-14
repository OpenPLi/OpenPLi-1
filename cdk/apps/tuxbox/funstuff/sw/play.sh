#!/bin/sh
modprobe gtx_fb
fbset -match -xres 640 -yres 480 -vxres 640 -vyres 480
./player > /dev/console
