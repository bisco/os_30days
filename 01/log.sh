#!/bin/bash

dd if=/dev/zero of=./hello_dummy.img bs=`printf "%d" 0x168000` count=1
# あとはvimで編集。/mnt/md0/grape/work/30jisaku/projects/01_day/helloos0/helloos.imgを写経。
# vim conv_binfile_to_nasm.py

# ディスクイメージを生成
python3 conv_binfile_to_nasm.py hello.img > hello_generated.s
nasm -f bin -o hello2.img hello_generated.s

# ディスクイメージを生成するアセンブラスクリプト(コメント付き)
#vim hello.s
nasm -f bin -o hello_by_asm.img hello.s

