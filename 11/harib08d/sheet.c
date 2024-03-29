#include "bootpack.h"

#define SHEET_USE       (1)

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if(ctl == 0) { //メモリ確保失敗
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1; // シートは1枚もない
    for(i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0; // 未使用マーク
        ctl->sheets0[i].ctl = ctl; // 所属を記録
    }
err:
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
    struct SHEET *sht;
    int i;
    for(i = 0; i < MAX_SHEETS; i++) {
        if(ctl->sheets0[i].flags == 0) {
            sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE; // 使用中マーク
            sht->height = -1;       // 非表示中
            return sht;
        }
    }
    return 0;   // 全て使用中
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

// vx0, vy0: 描画開始位置
// vx1, vy1: 描画終了位置
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1) {
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, c, *vram = ctl->vram;
    struct SHEET *sht;
    if(vx0 < 0) { vx0 = 0; }
    if(vy0 < 0) { vy0 = 0; }
    if(vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if(vy1 > ctl->xsize) { vy1 = ctl->ysize; }
    for(h = 0; h <= ctl->top; h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        // vx0～vy1を使って、bx0～by1を逆算する
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if(bx0 < 0) { bx0 = 0; }
        if(by0 < 0) { by0 = 0; }
        if(bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if(by1 > sht->bysize) { by1 = sht->bysize; }
        for(by = by0; by < by1; by++) {
            vy = sht->vy0 + by;
            for(bx = bx0; bx < bx1; bx++) {
                vx = sht->vx0 + bx;
                c = buf[by * sht->bxsize + bx];
                if(c != sht->col_inv) {
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
    return;
}

void sheet_updown(struct SHEET *sht, int height) {
    struct SHTCTL *ctl = sht->ctl;
    int h, old = sht->height; // 設定前の高さを記憶
    if(height > ctl->top + 1) {
        height = ctl->top + 1;
    }
    if(height < -1) {
        height = -1;
    }
    sht->height = height;

    // 以下はsheets[]の並べ替え
    if(old > height) { // 以前よりも低い
        if(height >= 0) {
            // 間のものを引き上げる
            for(h = old; h > height; h--) {
                ctl->sheets[h] = ctl->sheets[h-1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else { // 非表示化
            if(ctl->top > old) { // 上になっているものをおろす
                for(h = old; h < ctl->top; h++) {
                    ctl->sheets[h] = ctl->sheets[h+1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; // sheetが1つ減るので、高さがも1つ減る
        }
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
    } else if(old < height) { // 以前よりも高い
        if(old >= 0) { // 間のものを押し下げる
            for(h = old; h < height; h++) {
                ctl->sheets[h] = ctl->sheets[h+1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else { // 非表示→表示
            for(h = ctl->top; h >= height; h--) {
                ctl->sheets[h+1] = ctl->sheets[h];
                ctl->sheets[h+1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; // 表示枚数が増える
        }
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
    }
    return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
    if(sht->height >= 0) {
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
    }
    return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if(sht->height >= 0) { // 表示中
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
    }
    return;
}

void sheet_free(struct SHEET *sht) {
    struct SHTCTL *ctl = sht->ctl;
    if(sht->height >= 0) {
        sheet_updown(sht, -1); // まずは非表示に
    }
    sht->flags = 0;
    return;
}
