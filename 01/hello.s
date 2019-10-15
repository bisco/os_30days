; fat12 format
db    0xeb, 0x4e, 0x90
db    "HELLOIPL"        ; ブートセクタの名前(8bytes)
dw    512               ; size of 1 sector (word = 2バイト)
db    1                 ; クラスタの大きさ
dw    1                 ; FATの開始地点(普通は1セクタ目から)
db    2                 ; FATの個数
dw    224               ; ルートディレクトリ領域の大きさ(普通は224エントリにする)
dw    2880              ; ドライブの大きさ(2880セクタ = 512 * 2880 Bytes = 1474560 Bytes)
db    0xf0              ; メディアのタイプ(0xf0にしなければならない)
dw    9                 ; FAT領域の長さ
dw    18                ; 1トラックにいくつのセクタがあるか？(18にしなければならない)
dw    2                 ; ヘッドの数(2にしなければならない)
dd    0                 ; パーティションを使っていないので0
dd    2880              ; もう一度ドライブの大きさ
db    0, 0, 0x29        ; マジックナンバー
dd    0xffffffff        ; ボリュームシリアル番号らしい(double word = 4バイト)
db    "HELLO-OS   "     ; ディスクの名前(11Bytes)
db    "FAT12   "        ; フォーマットの名前(8Bytes)
times 18 db 0           ; 18Bytesあけておく

;プログラム本体
db    0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
db    0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
db    0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
db    0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
db    0xee, 0xf4, 0xeb, 0xfd

;メッセージ部分
db    0x0a, 0x0a        ; 改行を2つ
db    "hello, bisco os"
db    0x0a              ; 改行
db    0x0

times 0x1fe-($-$$) db 0 ; 0x01feまでを0x0で埋める
                        ; ($-$$)は現在の位置までのバイト数を求めるnasmのイディオム。
                        ; gasでやる場合はラベルを使う。

db    0x55, 0xaa        ; これはブート可能なセクタであることをBIOSに知らせるマジックワード

;ブートセクタ以外の記述
db    0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
times 4600 db 0
db    0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
times 1469432 db 0
