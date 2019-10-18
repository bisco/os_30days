// 割り込み関係

#include "bootpack.h"

void init_pic(void) {
    io_out8(PIC0_IMR, 0xff); // 全ての割り込みを受け付けない
    io_out8(PIC1_IMR, 0xff); // 全ての割り込みを受け付けない

    io_out8(PIC0_ICW1, 0x11); // エッジトリガモード
    io_out8(PIC0_ICW2, 0x20); // IRQ0-7は、INT20-27で受ける
    io_out8(PIC0_ICW3, 1 << 2); // PIC1はIRQ2にて接続(そういう仕様)
    io_out8(PIC0_ICW4, 0x01); // ノンバッファモード

    io_out8(PIC1_ICW1, 0x11); // エッジトリガモード
    io_out8(PIC1_ICW2, 0x28); // IRQ8-15は、INT28-2fで受ける
    io_out8(PIC1_ICW3, 2); // PIC1はIRQ2にて接続(そういう仕様)
    io_out8(PIC1_ICW4, 0x01); // ノンバッファモード

    io_out8(PIC0_IMR, 0xfb); // 11111011 PIC1以外は全て割込み禁止
    io_out8(PIC1_IMR, 0xff); // 11111111 全ての割り込みを禁止
}

void inthandler21(int *esp) {
    // PS/2キーボードからの割り込み
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 Keyboard");
    for(;;) {
        io_hlt();
    }
}

void inthandler2c(int *esp) {
    // PS/2マウスからの割り込み
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
    for(;;) {
        io_hlt();
    }
}

void inthandler27(int *esp) {
    // PIC0からの不完全割り込み対策
    // 電気的ノイズで発生するものらしいので、
    // 単に完了通知を出せば良い
    io_out8(PIC0_OCW2, 0x67); // IRQ7受付完了をPICに通知
    return;
}
