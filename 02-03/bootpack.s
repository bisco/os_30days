	.file	"bootpack.c"
	.intel_syntax noprefix
	.text
	.globl	HariMain
	.type	HariMain, @function
HariMain:
.LFB0:
	.cfi_startproc
	sub	rsp, 8
	.cfi_def_cfa_offset 16
.L2:
	call	io_hlt@PLT
	jmp	.L2
	.cfi_endproc
.LFE0:
	.size	HariMain, .-HariMain
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits
