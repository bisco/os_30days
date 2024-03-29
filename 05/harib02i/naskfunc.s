;[FORMAT "WCOFF"]                ; オブジェクトファイルを作るモード。nasmでは不要。
[BITS 32]                       ; 32bitモード用の機械語を作らせる

; オブジェクトファイルのための情報
;[FILE "naskfunc.s"]             ; ソースファイル名情報。nasmでは不要。
        GLOBAL  io_hlt, io_cli, io_sti, io_stihlt
        GLOBAL  io_in8, io_in16, io_in32
        GLOBAL  io_out8, io_out16, io_out32
        GLOBAL  io_load_eflags, io_store_eflags
        GLOBAL  load_gdtr, load_idtr

; 実際の巻数
[SECTION .text]     ; オブジェクトファイルではこれを書いてからプログラムを書く
io_hlt:    ; void io_hlt(void);
        HLT
        RET

io_cli:
        CLI
        RET

io_sti:
        STI
        RET

io_stihlt:
        STI
        HLT
        RET

io_in8:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     EAX, 0
        IN      AL, DX
        RET

io_in16:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     EAX, 0
        IN      AX, DX
        RET

io_in32:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     EAX, 0
        IN      EAX, DX
        RET

io_out8:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     AL, [ESP+8]         ; data指定
        OUT     DX, AL
        RET

io_out16:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     EAX, [ESP+8]        ; data指定
        OUT     DX, AX
        RET

io_out32:
        MOV     EDX, [ESP+4]        ; port指定
        MOV     EAX, [ESP+8]        ; data指定
        OUT     DX, EAX
        RET

io_load_eflags:
        PUSHFD
        POP EAX
        RET

io_store_eflags:
        MOV     EAX, [ESP+4]
        PUSH    EAX
        POPFD
        RET

load_gdtr:      ; void load_gdtr(int limit, int addr);
        MOV     AX, [ESP + 4]       ; limit
        MOV     [ESP + 6], AX
        LGDT    [ESP + 6]
        RET

load_idtr:      ; void load_idtr(int limit, int addr);
        MOV     AX, [ESP + 4]       ; limit
        MOV     [ESP + 6], AX
        LIDT    [ESP + 6]
        RET
