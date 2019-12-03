TARGET = haribote
OBJS_BOOTPACK = bootpack.o naskfunc.o graphic.o dsctbl.o int.o \
				hankaku.o sprintf.o strcmp.o fifo.o \
				keyboard.o mouse.o \
				memory.o sheet.o timer.o \
				mtask.o

.PHONY = clean diff default

sys_file_size = $(shell wc -c $(TARGET).sys | perl -lane 'print int($$F[0]/512 + 0.5) + 1')

# デフォルト
default:
	make $(TARGET).img

# ファイル生成規則
$(TARGET).img : ipl10.bin $(TARGET).sys
	mformat -f 1440 -C -B ipl10.bin -i $(TARGET).img ::
	mcopy $(TARGET).sys -i $(TARGET).img ::
	mcopy ipl10.s -i $(TARGET).img ::
	mcopy Makefile -i $(TARGET).img ::


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
