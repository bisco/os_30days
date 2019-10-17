TARGET = haribote

.PHONY = clean diff

sys_file_size = $(wc -c $(TARGET).sys | perl -lane 'print $F[0]/512 + 1')

$(TARGET).img : ipl10.bin $(TARGET).sys
	echo $(TARGET).sys > $(TARGET).name
	dd if=ipl10.bin of=$(TARGET).img count=2880 bs=512 conv=notrunc
	dd if=$(TARGET).name of=$(TARGET).img count=1 bs=512 seek=19 conv=notrunc # file名は0x2600バイト目に入れる
	dd if=$(TARGET).sys of=$(TARGET).img count=15 bs=512 seek=33 conv=notrunc # プログラムは0x4200バイト目から

%.o:%.c
	gcc -m32 -c -fno-pic -o $@ $<
	objdump -D $@ > $@.dmp

%.bin : %.oT
	objcopy -O binary $^ $@

ipl10.bin: ipl10.s
	nasm -f bin -o $@ $^ -l $^.lst

asmhead.o : asmhead.s
	nasm -o $@ $^ -l $^.lst

naskfunc.o : naskfunc.s
	nasm -felf32 -o $@ $^ -l $^.lst

$(TARGET).bin : bootpack.o naskfunc.o hankaku.o
	ld -m elf_i386 -e HariMain -n -Thrb.ld -static -o $(TARGET).bin $^
	hexdump -C $(TARGET).bin > $(TARGET).bin.dmp

$(TARGET).sys : asmhead.o $(TARGET).bin
	cat $^ > $@

hankaku.c : hankaku.txt
	./gen_hankaku.py hankaku.txt > hankaku.c

diff: $(TARGET).diff haribote_reference.diff
	diff -y -W200 $^

%.diff: %.img
	hexdump -C $^ > $@

clean:
	@rm -rf *.sys $(TARGET).img *.o *.bin *.lst *.name *.obj *.dmp bootpack.s hankaku.c
