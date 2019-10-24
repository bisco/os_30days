#include "bootpack.h"

#define MEMMAN_FREES        (4090)  // 約32KB
#define MEMMAN_ADDR         (0x003c0000)

struct FREEINFO {
    unsigned int addr, size;
};

struct MEMMAN {
    int frees, maxfrees, lostsize, losts;
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void) {
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    char s[40], msg[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    struct MOUSE_DEC mdec;
    unsigned int memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

    init_gdtidt();
    init_pic();
    io_sti(); // IDT/PICの初期化が終わったのでCPUの割込み禁止を解除
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); // PIC1とキーボードを許可
    io_out8(PIC1_IMR, 0xef); // マウスを許可
    init_keyboard();
    enable_mouse(&mdec);

    memtotal = memtest(0x00f00000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); // 0x00001000 - 0x0009efffまで使える
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    //putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello nbisco OS");
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

    mysprintf(s, "memory %dMB     free : %dKB",
            memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    for(;;) {
        io_cli();
        if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else {
            if(fifo8_status(&keyfifo) != 0) {
                i = fifo8_get(&keyfifo);
                io_sti();
                mysprintf(s, "%x", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if(fifo8_status(&mousefifo) != 0) {
                i = fifo8_get(&mousefifo);
                io_sti();
                if(mouse_decode(&mdec, i) != 0) {
                    mysprintf(s, "[lcr %d %d]", mdec.x, mdec.y);
                    if((mdec.btn & 0x01) != 0) { s[1] = 'L'; }
                    if((mdec.btn & 0x02) != 0) { s[3] = 'R'; }
                    if((mdec.btn & 0x04) != 0) { s[2] = 'C'; }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    // マウスカーソルの移動
                    // 一旦マウス消す
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
                    mx += mdec.x;
                    my += mdec.y;
                    if(mx < 0) { mx = 0; }
                    if(my < 0) { my = 0; }
                    if(mx > binfo->scrnx - 16) { mx = binfo->scrnx - 16; }
                    if(my > binfo->scrny - 16) { my = binfo->scrny - 16; }
                    mysprintf(s, "(%d, %d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); // 座標消す
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); // 座標書く
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // マウス描く
                }
            }
        }
    }
}

#define EFLAGS_AC_BIT       (0x00040000)
#define CR0_CACHE_DISABLE   (0x60000000)

unsigned int memtest_sub(unsigned int start ,unsigned int end) {
    unsigned int i, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    volatile unsigned int *p; // コンパイラに消されないように
    for(i = start; i <= end; i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;           // 値を覚えておく。
        *p = pat0;          // 書いてみる
        *p ^= 0xffffffff;   // 反転
        if(*p != pat1) {    // 反転結果になったか？
not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff;   // もう一度反転
        if(*p != pat0) {    //もとに戻った？
            goto not_memory;
        }
        *p = old;
    }
    return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    // 386(キャッシュなし) or 486(キャッシュあり)以降の確認
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0) { // 386ではAC=1にしても0に戻る
        // AC = Alignment Check
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    if(flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if(flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return i;

}

void memman_init(struct MEMMAN *man) {
    man->frees = 0;     // 空き情報の個数
    man->maxfrees = 0;  // freesの最大値
    man->lostsize = 0;  // 解放に失敗した合計サイズ
    man->losts = 0;     // 解放に失敗した合計サイズ
    return;
}

unsigned int memman_total(struct MEMMAN *man) {
    unsigned int i, t = 0;
    for(i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
    unsigned int i, a;
    for(i = 0; i < man->frees; i++) {
        if(man->free[i].size >= size) {
            // 空き発見
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if(man->free[i].size == 0) {
                // free[i]がなくなったので前へ詰める
                man->frees--;
                for(;i < man->frees; i++) {
                    man->free[i] = man->free[i + 1]; // 構造体の代入なのでセーフ
                }
            }
            return a;
        }
    }
    return 0; // 空きがなかった・・・
}

int memman_free(struct MEMMAN *man,unsigned int addr, unsigned int size) {
    int i, j;
    // まとめやすさを考えると、free[]がaddr順に並んでいる方がよいので、
    // まずはどこに入れるべきかを決める
    for(i = 0; i < man->frees; i++) {
        if(man->free[i].addr > addr) {
            break;
        }
    }

    // free[i-1].addr < addr < free[i].addr
    if(i > 0) {
        // 前がある
        if(man->free[i-1].addr + man->free[i-1].size == addr) {
            // 前の空き領域にまとめられる
            man->free[i-1].size += size;
            if(i < man->frees) {
                // 後ろもある
                if(addr + size == man->free[i].addr) {
                    // 後ろもまとめられる
                    man->free[i-1].size += man->free[i].size;
                    // man->free[i]の削除
                    man->frees--;
                    for(; i < man->frees; i++) {
                        man->free[i] = man->free[i+1];
                    }
                }
            }
            return 0;
        }
    }
    // 前とはまとめられなかった場合
    if(i < man->frees) {
        // しかし、後ろがある
        if(addr + size == man->free[i].addr) {
            // 後ろとはまとめられる
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0; // 成功
        }
    }
    // 前にも後ろにもまとめられなかった
    if(man->frees < MEMMAN_FREES) {
        // free[i]より後ろを後ろへずらして隙間を作る
        for(j = man->frees; j > i; j--) {
            man->free[j] = man->free[j-1];
        }
        man->frees++;
        if(man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }
    // 後ろにずらせなかった
    man->losts++;
    man->lostsize += size;
    return -1;
}

