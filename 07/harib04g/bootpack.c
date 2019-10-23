#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[4], msg[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;

    init_gdtidt();
    init_pic();
    io_sti(); // IDT/PICの初期化が終わったのでCPUの割込み禁止を解除
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);

    init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

    putfonts8_asc(binfo->vram, binfo->scrnx, 1, 1, COL8_000000, "Hello nbisco OS");
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello nbisco OS");

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    mysprintf(msg, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 18, COL8_FFFFFF, msg);

    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);
    init_keyboard();
    enable_mouse();

    for(;;) {
        io_cli();
        if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else {
            if(fifo8_status(&keyfifo) != 0) {
                i = fifo8_get(&keyfifo);
                io_sti();
                mysprintf(s, "%x", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 48, 15, 48+15);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 48, COL8_FFFFFF, s);
            } else if(fifo8_status(&mousefifo) != 0) {
                i = fifo8_get(&mousefifo);
                io_sti();
                mysprintf(s, "%x", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 64, 15, 64+15);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 64, COL8_FFFFFF, s);
            }
        }
    }
}

#define PORT_KEYDAT             (0x0060)
#define PORT_KEYSTA             (0x0064)
#define PORT_KEYCMD             (0x0064)
#define KEYSTA_SEND_NOTREADY    (0x02)
#define KEYCMD_WRITE_MODE       (0x60)
#define KBC_MODE                (0x47)

void wait_KBC_sendready(void) {
    // キーボードコントローラがデータ送信可能になるのを待つ
    for(;;) {
        if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(void) {
    // キーボードコントローラの初期化
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
    return;
}

#define KEYCMD_SENDTO_MOUSE     (0xd4)
#define MOUSECMD_ENABLE         (0xf4)

void enable_mouse(void) {
    // マウス有効化
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    return; // うまくいくとACK(0xfa)が返ってくる
}
