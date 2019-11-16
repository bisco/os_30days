CYLS    EQU     10
        ORG     0x7c00          ; このプログラムの読み込まれる位置

; FAT12フォーマットフロッピーディスクのための記述
        JMP     entry
        DB      0x90
        DB      "HARIBOTE"      ; ブートセクタ名(8バイト)
        DW      512             ; 1セクタの大きさ
        DB      1               ;
        DW      1               ; FATのスタート位置
        DB      2               ; FATの個数(2にしなければならない)
        DW      224             ;
        DW      2880
        DB      0xf0            ; メディアのタイプ(0xF0にしなければならない)
        DW      9               ;
        DW      18              ;
        DW      2               ;
        DD      0               ;
        DD      2880            ;
        DB      0,0,0x29        ;
        DD      0xffffffff      ;
        DB      "HARIBOTEOS "   ; ディスクの名前
        DB      "FAT12   "      ;
        TIMES 18 DB 0           ; 18バイトあけておく(とりあえず)

; プログラム本体
entry:
        MOV     AX, 0           ; レジスタ初期化
        MOV     SS, AX          ; SS = Stack Segment(今は使わないっぽい)
        MOV     SP, 0x7c00
        MOV     DS, AX          ; DS = Data Segment(今は使わないっぽい)

; ディスクを読む
        MOV     AX, 0x0820      ; 0x08200に読んだデータを置く
        MOV     ES, AX          ; ES x16 + BX = 0x8200
        MOV     CH, 0           ; シリンダ0
        MOV     DH, 0           ; ヘッド0
        MOV     CL, 2           ; セクタ2。ブートローダのあとを読む。
readloop:
        MOV     SI, 0           ; SI = Source Index。今は失敗回数を数えるレジスタ。
retry:
        MOV     AH, 0x02        ; AH = 0x02 -> ディスク読み込み
        MOV     AL, 1           ; 1セクタ読む
        MOV     BX, 0
        MOV     DL, 0x00        ; Aドライブ(DL = ドライブ番号)
        INT     0x13            ; ディスクBIOS呼び出し
        JNC     next            ; エラーが起きなければnextへ
        ADD     SI, 1           ; SI(失敗回数カウントレジスタ)に1たす
        CMP     SI, 5           ; 5回？
        JAE     error           ; 5回以上ならerrorへ
        MOV     AH, 0x00
        MOV     DL, 0x00
        INT     0x13            ; ドライブのリセット
        JMP     retry
next:
        MOV     AX, ES          ; アドレスを0x200 = 512バイトすすめる
        ADD     AX, 0x0020
        MOV     ES, AX
        ADD     CL, 1
        CMP     CL, 18          ; 1シリンダ、1ヘッドあたり18セクタ
        JBE     readloop
        MOV     CL, 1
        ADD     DH, 1
        CMP     DH, 2
        JB      readloop
        MOV     DH, 0
        ADD     CH, 1
        CMP     CH, CYLS
        JB      readloop

; 読み終わった
        MOV     [0x0ff0], CH
        JMP     0xc200

error:
        MOV     SI, msg
putloop:
        MOV     AL, [SI]
        ADD     SI, 1
        CMP     AL, 0
        JE      fin
        MOV     AH, 0x0e        ; 1文字表示ファンクション
        MOV     BX, 10          ; カラーコード
        INT     0x10            ; ビデオBIOS呼び出し
        JMP     putloop
fin:
        HLT
        JMP     fin             ; 無限ループ
msg:
        DB      0x0a, 0x0a      ; 改行2つ
        DB      "load error"
        DB      0x0a
        DB      0

        TIMES 0x1fe-($-$$) DB 0

        DB      0x55, 0xaa
        DB      0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00
        TIMES 4600 DB 0
        DB      0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00
        TIMES 1469432 DB 0
