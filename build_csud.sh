#!/bin/sh
cd csud
make clean
CFLAGS="-mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s" make driver CONFIG=DEBUG TYPE=LOWLEVEL TARGET=RPI GNU=arm-none-eabi- LIB_KBD=1 LIB_MOUSE=0
