#include <stdarg.h>

//10進数からASCIIコードに変換
int dec2asc (char *str, int dec, char fill, int fill_times) {
    int len = 0, len_buf; //桁数
    int buf[30];
    int minus = 0;
    int i;
    if(dec < 0) {
        dec = -dec;
        minus = 1;
    }
    while (1) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
        buf[len++] = dec % 10;
        if (dec < 10) break;
        dec /= 10;
    }
    for(;fill_times > len;) {
        buf[len++] = fill - 0x30;
    }
    if(minus) {
        buf[len++] = '-' - 0x30;
    }
    len_buf = len;
    while (len) {
        *(str++) = buf[--len] + 0x30;
    }
    return len_buf;
}

//16進数からASCIIコードに変換
int hex2asc (char *str, int dec, char fill, int fill_times) { //16で割れた回数（つまり桁数）をlenに、各桁をbufに格納
    int len = 0, len_buf; //桁数
    int buf[10];
    int minus = 0;
    if(dec < 0) {
        dec = -dec;
        minus = 1;
    }
    while (1) {
        buf[len++] = dec % 16;
        if (dec < 16) break;
        dec /= 16;
    }
    for(;fill_times > len;) {
        buf[len++] = fill - 0x30;
    }
    if(minus) {
        buf[len++] = '-' - 0x30;
    }
    len_buf = len;
    while (len) {
        len --;
        *(str++) = (buf[len]<10)?(buf[len] + 0x30):(buf[len] - 9 + 0x60);
    }
    return len_buf;
}

void mysprintf (char *str, char *fmt, ...) {
    va_list list;
    char fill;
    char count[5];
    int i, j, len, fill_times = 0;
    va_start (list, fmt);

    while (*fmt) {
        if(*fmt=='%') {
            fmt++;
            switch(*fmt) {
            case 'd':
            case 'x':
                break;
            default:
                fill = *(fmt++);
                for(i=0;;i++) {
                    count[i] = *(fmt++);
                    if((*fmt == 'd') || (*fmt == 'x')) break;
                }
                for(j = 0; i >= 0; i--, j++) {
                    fill_times += 10 * j * (count[i] - '0');
                }
                break;
            }
            switch(*fmt){
                case 'd':
                    len = dec2asc(str, va_arg (list, int), fill, fill_times);
                    break;
                case 'x':
                    len = hex2asc(str, va_arg (list, int), fill, fill_times);
                    break;
            }
            str += len; fmt++;
        } else {
            *(str++) = *(fmt++);
        }
    }
    *str = 0x00; //最後にNULLを追加
    va_end (list);
}
