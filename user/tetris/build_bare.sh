#!/bin/sh
arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -Wl,-T,link.ld -std=c99 -O2 api_bare.c main.c -lm
