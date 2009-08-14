#!/bin/sh
make
./easylogo linux_logo.tga ppcboot_logo video_logo.h
mv video_logo.h ../../include
