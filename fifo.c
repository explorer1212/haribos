/*
 * @Date: 2023-01-03 13:45:12
 * @LastEditTime: 2023-01-10 10:20:42
 * @FilePath: \helloos0\fifo.c
 * @Description: 
 * 
 */
#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; 
	fifo->flags = 0;
	fifo->p = 0; 
	fifo->q = 0; 
	fifo->task = task;
	return;
}

/**
 * @description: put a data in fifo
 * @param {FIFO8} *fifo
 * @param {unsigned char} data
 * @return {*}
 */
int fifo32_put(struct FIFO32 *fifo, int data)
{
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { /* is sleeping */
			task_run(fifo->task, -1, 0); /* wakeup it */
		}
	}
	return 0;
}

/**
 * @description: get a data in fifo
 * @param {FIFO8} *fifo
 * @return {*}
 */
int fifo32_get(struct FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	// char *s;
	// sprintf(s, "%03X", data);
	// putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 3);
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

/**
 * @description: used size
 * @param {FIFO8} *fifo
 * @return {*}
 */
int fifo32_status(struct FIFO32 *fifo)
{
	return fifo->size - fifo->free;
}
