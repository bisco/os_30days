;[FORMAT "WCOFF"]                ; オブジェクトファイルを作るモード。nasmでは不要。
[BITS 32]                       ; 32bitモード用の機械語を作らせる

; オブジェクトファイルのための情報
;[FILE "naskfunc.s"]             ; ソースファイル名情報。nasmでは不要。
        GLOBAL  io_hlt           ; このプログラムに含まれる関数名

; 実際の巻数
[SECTION .text]     ; オブジェクトファイルではこれを書いてからプログラムを書く
io_hlt:    ; void io_hlt(void);
        HLT
        RET
