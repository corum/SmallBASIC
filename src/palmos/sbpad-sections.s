/* DO NOT EDIT!
   This file was automatically generated by m68k-palmos-multigen v1.2
   from sbpad.def  */

	.file	"sbpad-sections.s"

	.globl	__text__
	.lcomm	__text__,4
	.globl	__text__BCSCAN
	.lcomm	__text__BCSCAN,4
	.globl	__text__BCSC2
	.lcomm	__text__BCSC2,4
	.globl	__text__BCSC3
	.lcomm	__text__BCSC3,4
	.globl	__text__BEXEC
	.lcomm	__text__BEXEC,4
	.globl	__text__TRASH
	.lcomm	__text__TRASH,4
	.globl	__text__IDE
	.lcomm	__text__IDE,4
	.globl	__text__BLIB
	.lcomm	__text__BLIB,4
	.globl	__text__BMATH
	.lcomm	__text__BMATH,4
	.globl	__text__BMATH2
	.lcomm	__text__BMATH2,4
	.globl	__text__BIO
	.lcomm	__text__BIO,4
	.globl	__text__BIO2
	.lcomm	__text__BIO2,4
	.globl	__text__BIO3
	.lcomm	__text__BIO3,4
	.globl	__text__PALMFS
	.lcomm	__text__PALMFS,4

.text
	.globl	_GccRelocateData
_GccRelocateData:
	bra.w	_GccLoadCodeAndRelocateData

.section ehook
	.long	_GccReleaseCode
