#!/bin/sh
arm-none-eabi-objdump -x target/arm-unknown-linux-gnueabihf/debug/rs | head -n 40
