#include "bootpack.h"

struct MOUSE_DEC {
    unsigned char buf[3], phase;
    int x, y, btn;
};

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[40], msg[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    struct MOUSE_DEC mdec;

    init_gdtidt();
    init_pic();
    io_sti(); // IDT/PICの初期化が終わったのでCPUの割込み禁止を解除
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);

    init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

    putfonts8_asc(binfo->vram, binfo->scrnx, 1, 1, COL8_000000, "Hello nbisco OS - 1");
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello nbisco OS - 1");

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);
    init_keyboard();
    enable_mouse(&mdec);

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
                if(mouse_decode(&mdec, i) != 0) {
                    mysprintf(s, "[lcr %d %d]", mdec.x, mdec.y);
                    if((mdec.btn & 0x01) != 0) {
                        s[1] = 'L';
                    }
                    if((mdec.btn & 0x02) != 0) {
                        s[3] = 'R';
                    }
                    if((mdec.btn & 0x04) != 0) {
                        s[2] = 'C';
                    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 16 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    // マウスカーソルの移動
                    // 一旦マウス消す
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
                    mx += mdec.x;
                    my += mdec.y;
                    if(mx < 0) {
                        mx = 0;
                    }
                    if(my < 0) {
                        my = 0;
                    }
                    if(mx > binfo->scrnx - 16) {
                        mx = binfo->scrnx - 16;
                    }
                    if(my > binfo->scrny - 16) {
                        my = binfo->scrny - 16;
                    }
                    mysprintf(s, "(%d, %d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 32, 79, 32 + 15); // 座標消す
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s); // 座標書く
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // マウス描く
                }
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

void enable_mouse(struct MOUSE_DEC *mdec) {
    // マウス有効化
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
    mdec->phase = 0;
    return; // うまくいくとACK(0xfa)が返ってくる
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
    if(mdec->phase == 0) {
        // ACKを読み捨てる
        if(dat == 0xfa) {
            mdec->phase = 1;
        }
        return 0;
    }
    if(mdec->phase == 1) {
        // 1バイト目を待っている
        if((dat & 0xc8) == 0x08) {
            // 正しい1バイト目だった
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if(mdec->phase == 2) {
        // 2バイト目を待っている
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if(mdec->phase == 3) {
        // 3バイト目を待っている
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        if((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xffffff00;
        }
        if((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -mdec->y;
        return 1;
    }
    return -1; // ここにはこない


}
