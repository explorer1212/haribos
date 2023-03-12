# HariOS

进入32位保护模式，进入内核后，初始化GDT、IDT、PIC，打开中断。

## 目录结构

### hankaku.txt

保存字体

### bootpack.h

头文件

### asmhead.nas

启动内核的汇编程序，0x7c00

### naskfunc.nas

实现无法用C语言直接编写的函数。

### graphic.c

用来显示、打印的文件

```c
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
/* print a string */
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
/* print the mouse */
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, char *buf, int bxsize);	
#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15
```

### dsctbl.c

与GDT、IDT有关

```c
/* GDT structure */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
/* IDT structure */
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
/* init gdt and idt */
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800 /* address of IDT */
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000 /* address of GDT */
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000 /* address of bootpack.h */
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define	AR_TSS32		0x0089
#define AR_INTGATE32	0x008e
```



## 功能

### 1、中断

以_asm_handler21为例，将函数注册到IDT中，当遇到中断时，PIC向CPU发送2个字节的数据0xcd,0xxx，0xcd就是INT，以此让CPU把数据当作命令来执行来产生中断，调用这个汇编函数。该汇编函数保存好寄存器并设置好DS、ES段寄存器后，调用\_handler21（C程序）来处理程序。








## 函数调用路径

### 应用程序

命令行输入命令后(cons_runcmd) -> cmd_app -> _start_app

### 中断

api -> 通过INT 0x40 -> _asm_hrb_api -> hrb_api

### 窗口切换



## API

关闭窗口 shift f1 或者 点击x号。

## 命令

mem

dir

type xxx

用户程序

walk.c

lines.c



