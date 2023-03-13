# HariOS

​	进入32位保护模式，进入内核后，初始化GDT、IDT、PIC，打开中断。

## 功能

### 1、中断

​	以_asm_handler21为例，将函数注册到IDT中，当遇到中断时，PIC向CPU发送2个字节的数据0xcd,0xxx，0xcd就是INT，以此让CPU把数据当作命令来执行来产生中断，调用这个汇编函数。该汇编函数保存好寄存器并设置好DS、ES段寄存器后，调用\_handler21（C程序）来处理程序。

#### 键盘中断

​	向0x64读数据，看位1是否为0，为0代表键盘控制电路准备完毕，然后进行模式设定。向0x60端口写入0x47，代表利用鼠标模式。

##### 键盘信息的解码

#### 鼠标中断

​	向键盘控制电路发送指令0xd4，下一个数据会发送给鼠标，发送激活指令后，正确的话会返回0xfa。

##### 鼠标信息的解码

​	每次移动鼠标，数据包含3个字节。第0个字节与移动、点击还是滑轮有关，第1个字节与左右移动有关，第2个字节与上下移动有关。



## 目录结构

### hankaku.txt

保存字体

### bootpack.h

头文件

### asmhead.nas

启动内核的汇编程序，0x7c00，

#### 开启32位保护模式

打开A20地址线，

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

### keyboard.c

```c
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064
```

### mouse.c

```c
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
```

### fifo.c

缓冲区，用来存储中断产生的信息等等。

```c
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);
```



#### 










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



