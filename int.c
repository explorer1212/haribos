#include "bootpack.h"
#include <stdio.h>

/* programmable interrupt controller 可编程中断控制器 */
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* IMR是中断屏蔽寄存器，值为1代表屏蔽，禁止主PIC的中断 */
	io_out8(PIC1_IMR,  0xff  ); /* 禁止从PIC的中断 */

	io_out8(PIC0_ICW1, 0x11  ); /* ICW是初始化控制数据，边沿触发模式 */
	io_out8(PIC0_ICW2, 0x20  ); /* 只有这个ICW2能改，IRQ0-7由INT 20-27接收 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1由IRQ2连接 */
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* 边沿触发模式 */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15由INT 28-2f接收 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1由IRQ2连接 */
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1以外全部禁止 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 禁止PIC1的所有中断 */

	return;
}

void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); 
	return;
}
