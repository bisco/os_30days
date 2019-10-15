#!/bin/bash

dd if=/dev/zero of=./hello_dummy.img bs=`printf "%d" 0x168000` count=1
# あとはvimで編集。/mnt/md0/grape/work/30jisaku/projects/01_day/helloos0/helloos.imgを写経。

