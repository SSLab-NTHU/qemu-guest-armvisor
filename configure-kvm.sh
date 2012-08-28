#
# Add by SSLab, NTHU
#
# Compile script for compiling QEMU for supporting ARMvisor
#
# This file is under GNU GPLv2 License
#
#

#!/bin/bash

SOURCE_DIR=./
ZLIB_PREBUILT=`pwd`/package/zlib-1.2.3-arm

TARGET_LIST=arm-softmmu
CROSS_PREFIX=arm-none-linux-gnueabi-
CPU=armv4l
CFLAGS=-I$ZLIB_PREBUILT/include
LDFLAGS=-L$ZLIB_PREBUILT/lib
PREFIX=$HOME/sslab/kvm/sandbox/qemu-guest/qemu-0.15.1
OPT_FLAGS="--disable-user --disable-sdl --enable-io-thread --disable-guest-agent --disable-vhost-net"

DEBUG=--disable-strip
STATIC=--static

$SOURCE_DIR/configure --target-list=$TARGET_LIST --cpu=$CPU --extra-cflags=$CFLAGS --extra-ldflags=$LDFLAGS --cross-prefix=$CROSS_PREFIX $OPT_FLAGS --prefix=$PREFIX $DEBUG
