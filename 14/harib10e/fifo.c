// FIFOライブラリ

#include "bootpack.h"

#define FLAGS_OVERRUN   (0x0001)

void fifo32_init(struct FIFO32 *fifo, int size, int *buf) {
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size; // 空き
    fifo->flags = 0;
    fifo->p = 0; // p-index
    fifo->q = 0; // c-index
    return;
}

int fifo32_put(struct FIFO32 *fifo, int data) {
    if(fifo->free == 0) {
        // 空きがない
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    fifo->p %= fifo->size;
    fifo->free--;
    return 0;
}

int fifo32_get(struct FIFO32 *fifo) {
    int data;
    if(fifo->free == fifo->size) {
        // 空っぽ
        return -1;
    }
    data = fifo->buf[fifo->q];
    fifo->q++;
    fifo->q %= fifo->size;
    fifo->free++;
    return data;
}

int fifo32_status(struct FIFO32 *fifo) {
    return fifo->size - fifo->free;
}
