## Building the cross compiler on macOS

Get a working Free Pascal compiler first. Then download Free Pascal source code and run

```sh
tar xf fpc-3.0.4.source.tar.gz
cd fpc-3.0.4
make all OS_TARGET=embedded CPU_TARGET=arm SUBARCH=armv6 BINUTILSPREFIX=arm-none-eabi-
make crossinstall OS_TARGET=embedded CPU_TARGET=arm SUBARCH=armv6 BINUTILSPREFIX=arm-none-eabi-
ln -s /usr/local/lib/fpc/3.0.4/ppcrossarm /usr/local/bin/ppcarm
```
