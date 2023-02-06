/*
 * @Date: 2023-01-06 10:59:42
 * @LastEditTime: 2023-02-05 21:23:35
 * @FilePath: \helloos0\timer.c
 * @Description: 
 * 
 */
#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC   1
#define TIMER_FLAGS_USING    2

struct TIMERCTL timerctl;

void init_pit(void)
{
    int i;
    struct TIMER *t;
    /* 中断周期设定为11932(0x2e9c)， 
     in this case IRQ0 will generate 100 interrupts in 1s*/
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    timerctl.count = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = 0; /* unuse */
    }
    t = timer_alloc();
    t->timeout = 0xffffffff;
    t->flags = TIMER_FLAGS_USING;
    t->next = 0;
    timerctl.t0 = t;
    timerctl.next = 0xffffffff;
    return;
}

struct TIMER *timer_alloc(void)
{
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers0[i].flags == 0) {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            timerctl.timers0[i].flags2 = 0;
            return &timerctl.timers0[i];
        }
    }
    return 0;
}

void timer_free(struct TIMER *timer)
{
    timer->flags = 0;
    return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    int e;
    struct TIMER *t, *s;
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    t = timerctl.t0;
    if (timer->timeout <= t->timeout) {
        /* insert at the front */
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next = timer->timeout;
        io_store_eflags(e);
        return;
    }
    /* find where to insert */
    for (;;) {
        s = t;
        t = t->next;
        if (timer->timeout <= t->timeout) {
            /* insert between s and t */
            s->next = timer;
            timer->next = t;
            io_store_eflags(e);
            return;
        }
    }
    return;
}

void inthandler20(int *esp)
{
    struct TIMER *timer;
    char ts = 0;
    io_out8(PIC0_OCW2, 0x60);
    timerctl.count++;
    if (timerctl.next > timerctl.count) { /* timer.timeout > timerctl.count, return */
        return;
    }
    timer = timerctl.t0;
    for (;;) {
        if (timer->timeout > timerctl.count) {
            break;
        }
        /* timeout */
        timer->flags = TIMER_FLAGS_ALLOC;
        if (timer != task_timer) {
            fifo32_put(timer->fifo, timer->data);
        } else {
            ts = 1;
        }
        timer = timer->next;
    }
    timerctl.t0 = timer;
    timerctl.next = timerctl.t0->timeout;
    if (ts != 0) {
        task_switch(); /* the current task is timeout, so switch it */
    }
    return;
}

int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* �ݒ蒆�Ƀ^�C�}�̏�Ԃ��ω����Ȃ��悤�ɂ��邽�� */
	if (timer->flags == TIMER_FLAGS_USING) {	/* �����������͕K�v���H */
		if (timer == timerctl.t0) {
			/* �擪�������ꍇ�̎��������� */
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			/* �擪�ȊO�̏ꍇ�̎��������� */
			/* timer�̈�O��T�� */
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next; /* �utimer�̒��O�v�̎����A�utimer�̎��v���w���悤�ɂ��� */
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;	/* �L�����Z���������� */
	}
	io_store_eflags(e);
	return 0; /* �L�����Z�������͕s�v������ */
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();	/* �ݒ蒆�Ƀ^�C�}�̏�Ԃ��ω����Ȃ��悤�ɂ��邽�� */
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}
