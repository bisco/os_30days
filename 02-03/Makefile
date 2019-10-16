TARGET = haribote

.PHONY = clean diff

$(TARGET).img : ipl10.bin $(TARGET).sys
	echo $(TARGET).sys > $(TARGET).name
	dd if=ipl10.bin of=$(TARGET).img count=2880 bs=512 conv=notrunc
	dd if=$(TARGET).name of=$(TARGET).img count=1 bs=512 seek=19 conv=notrunc
	dd if=$(TARGET).sys of=$(TARGET).img count=4 bs=512 seek=33 conv=notrunc
bootpack.gas : bootpack.c
	$(CC) $(CFLAGS) $(C_ASMFLAGS) -o bootpack.s bootpack.c

%.o:%.c
	gcc -m32 -c -fno-pic -nostdlib -o $@ $<
	objdump -D $@ > $@.dmp

%.bin : %.oT
	objcopy -O binary $^ $@

ipl10.bin: ipl10.s
	nasm -f bin -o $@ $^ -l $^.lst

asmhead.o : asmhead.s
	nasm -o $@ $^ -l $^.lst

naskfunc.o : naskfunc.s
	nasm -felf32 -o $@ $^ -l $^.lst

$(TARGET).bin : bootpack.o naskfunc.o
	ld -m elf_i386 -e HariMain -n -Thrb.ld -static -o $(TARGET).bin $^
	hexdump -C $(TARGET).bin > $(TARGET).bin.dmp

$(TARGET).sys : asmhead.o $(TARGET).bin
	cat $^ > $@

diff: $(TARGET).diff haribote_reference.diff
	diff -y -W200 $^

%.diff: %.img
	hexdump -C $^ > $@

clean:
	@rm -rf *.sys $(TARGET).img *.o *.bin *.lst *.name *.obj *.dmp
