#include "bootpack.h"


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
        cr0 |= CR0_CACHE_DISABLE; // キャッシュ禁止
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if(flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // キャッシュ許可
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

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size) {
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000;
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size) {
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}
