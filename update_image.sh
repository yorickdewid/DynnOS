#!/bin/bash

/sbin/losetup /dev/loop0 floppy.img
mount /dev/loop0 tmp/
rm -f tmp/xex.dynnos
rm -f tmp/initrd
cp src/xex.dynnos tmp/
cp initrd.img tmp/initrd
umount /dev/loop0
/sbin/losetup -d /dev/loop0
