#!/bin/bash

# run_bochs.sh

echo c > tmp/no_debug
losetup /dev/loop0 floppy.img
bochs -qf bochsrc.txt -rc tmp/no_debug
losetup -d /dev/loop0
rm -f tmp/no_debug
