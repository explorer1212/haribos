; haribote-os boot asm
; TAB=4

[INSTRSET "i486p"]

VBEMODE	EQU		0x105			
;	0x100 :  640 x  400 x 8bit
;	0x101 :  640 x  480 x 8bit
;	0x103 :  800 x  600 x 8bit
;	0x105 : 1024 x  768 x 8bit
;	0x107 : 1280 x 1024 x 8bit

BOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; BOOT_INFO関係
CYLS	EQU		0x0ff0			; ブートセクタが設定する
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 色数に関する情報。何ビットカラーか？
SCRNX	EQU		0x0ff4			; address of resolution of x
SCRNY	EQU		0x0ff6			; address of resolution of y
VRAM	EQU		0x0ff8			; address of vram

		ORG		0xc200			

; check VBE mode

		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320


; check VBE version

		MOV		AX,[ES:DI+4]
		CMP		AX,0X0200
		JB		scrn320

; get screen infomation
	
		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; confirmation of screen mode infomation

		CMP		BYTE [ES:DI+0x19],8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]
		AND		AX,0x0080
		JZ		scrn320


; support VBE mode, set screen mode

		MOV		BX,VBEMODE+0x4000
		MOV 	AX,0X4F02
		INT		0x10
		MOV		BYTE [VMODE],8
		MOV		AX,[ES:DI+0x12] ; resolution of x
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14] ; resolution of y
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28] ; the address of vram
		MOV		[VRAM],EAX
		JMP		keystatus
	
scrn320:
		MOV		AL,0x13			
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000


keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; init PIC: close all interrupt

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						
		OUT		0xa1,AL

		CLI						

; 为了让CPU能够访问1MB以上的内存空间，设定A20GATE

		CALL	waitkbdout
        in al,0x92                                   ;南桥芯片内的端口
        or al,0000_0010B
        out 0x92,al                                  ;打开A20
		CALL	waitkbdout

; 切换到保护模式

		LGDT	[GDTR0]			
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设置bit31为0，禁止分页
		OR		EAX,0x00000001	; 设置bit0为1，为了切换到保护模式
		MOV		CR0,EAX
		JMP		pipelineflush	; 切换到保护模式时，必须要先执行JMP指令
pipelineflush:
		MOV		AX,1*8			; 可读写的段，32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的转送
; memcpy(bootpack, BOTPAK, 512*1024/4);
		MOV		ESI,bootpack	; source
		MOV		EDI,BOTPAK		; destination
		MOV		ECX,512*1024/4
		CALL	memcpy

; 磁盘数据最终转送它本来的位置去

; 首先从启动扇区开始
; memcpy(0x7c00, DSKCAC, 512/4);
		MOV		ESI,0x7c00		; source
		MOV		EDI,DSKCAC		; destination
		MOV		ECX,512/4
		CALL	memcpy

; 所有剩下的
; memcpy(DKSCAC0+512, DSKAC+512, CYLS*512*18*2/4 - 512/4);
		MOV		ESI,DSKCAC0+512	
		MOV		EDI,DSKCAC+512	
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	
		SUB		ECX,512/4		
		CALL	memcpy

; asmhead已完成它自己的工作
; 接下来启动bootpack

; bootpack的启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; completed
		MOV		ESI,[EBX+20]
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b ; 通过jmp指令，跳转到bootpack

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND的结果如果不是0，就跳到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET

		ALIGNB	16  ; 一直添加DBO，直到地址能被16整除
GDT0:
		RESB	8				
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以读写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以执行的段 32bit（bootpack用）

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
