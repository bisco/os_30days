TARGET = haribote
OBJS_BOOTPACK = bootpack.o naskfunc.o graphic.o dsctbl.o int.o \
				hankaku.o sprintf.o fifo.o \
				keyboard.o mouse.o \
				memory.o sheet.o timer.o

.PHONY = clean diff default

sys_file_size = $(shell wc -c $(TARGET).sys | perl -lane 'print int($$F[0]/512 + 0.5) + 1')

# デフォルト
default:
	make $(TARGET).img

# ファイル生成規則
$(TARGET).img : ipl10.bin $(TARGET).sys
	echo $(TARGET).sys > $(TARGET).name
	dd if=ipl10.bin of=$(TARGET).img count=2880 bs=512 conv=notrunc
	dd if=$(TARGET).name of=$(TARGET).img count=1 bs=512 seek=19 conv=notrunc # file名は0x2600バイト目に入れる
	dd if=$(TARGET).sys of=$(TARGET).img count=$(sys_file_size) bs=512 seek=33 conv=notrunc # プログラムは0x4200バイト目から

asmhead.o : asmhead.s
	nasm -o $@ $^ -l $^.lst

naskfunc.o : naskfunc.s
	nasm -felf32 -o $@ $^ -l $^.lst

$(TARGET).bin : $(OBJS_BOOTPACK)
	ld -m elf_i386 -e HariMain -n -Thrb.ld -static -o $(TARGET).bin $^
	hexdump -C $(TARGET).bin > $(TARGET).bin.dmp

$(TARGET).sys : asmhead.o $(TARGET).bin
	cat $^ > $@

hankaku.c : hankaku.txt
	./gen_hankaku.py hankaku.txt > hankaku.c

# 一般規則
%.o:%.c
	gcc -m32 -c -fno-pic -nostdlib -o $@ $<
	objdump -D $@ > $@.dmp

%.bin : %.oT
	objcopy -O binary $^ $@

ipl10.bin: ipl10.s
	nasm -f bin -o $@ $^ -l $^.lst

# PHONY
diff: $(TARGET).diff haribote_reference.diff
	diff -y -W200 $^

%.diff: %.img
	hexdump -C $^ > $@

clean:
	@rm -rf *.sys $(TARGET).img *.o *.bin *.lst *.name *.obj *.dmp bootpack.s hankaku.c
