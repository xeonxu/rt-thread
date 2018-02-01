/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtthread.h>
#include <drivers/pin.h>

static rt_sem_t sem = RT_NULL;
static rt_device_t device;
//rt_err_t result = RT_EOK;

struct rt_device_pin_mode sys_led[] =
{
    {.pin = 41, .mode = PIN_MODE_OUTPUT},
    {.pin = 54, .mode = PIN_MODE_OUTPUT},
};

struct rt_device_pin_status sys_led_status[] =
{
    {.pin = 41, .status = PIN_LOW},
    {.pin = 54, .status = PIN_HIGH},
};

static void thread_entry1(void* parameter)
{
    rt_uint8_t ch;
    while (1)
    {
	rt_sem_take(sem, RT_WAITING_FOREVER);
	for (ch = 'A';ch <= 'Z';ch++);
	/* rt_kprintf("ch:%c\n", ch); */
	rt_sem_release(sem);
	rt_thread_delay(50);
    }
}

static void thread_entry2(void* parameter)
{
    rt_uint32_t i;
    while (1)
    {
	rt_sem_take(sem, RT_WAITING_FOREVER);
	sys_led_status[0].status = !sys_led_status[1].status;
	rt_device_write(device, 0, &sys_led_status[0], sizeof(sys_led_status[0]));
	rt_sem_release(sem);
	rt_thread_delay(50);
    }
}


static void thread_entry3(void* parameter)
{
    while (1)
    {
	rt_sem_take(sem, RT_WAITING_FOREVER);
	sys_led_status[1].status = !sys_led_status[1].status;
	rt_device_write(device, 0, &sys_led_status[1], sizeof(sys_led_status[1]));
	rt_sem_release(sem);
	rt_thread_delay(50);
    }
}
        
int main(void)
{
    /* user app entry */
    rt_thread_t tid1;
    rt_thread_t tid2;
    rt_thread_t tid3;

    rt_uint8_t i;

    sem = rt_sem_create("sem", 1, RT_IPC_FLAG_FIFO); // ??????1
    if (sem == RT_NULL)
    {
	rt_kprintf("create semaphore failed\n");
	return -1;
    }

    device = rt_device_find("pin");
    if (RT_NULL != device) {
	rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
	for (i = 0; i < (sizeof(sys_led)/sizeof(sys_led[0])); ++i) {
	    rt_device_control(device, 0, &sys_led[i]);
	    rt_device_write(device, 0, &sys_led_status[i], sizeof(sys_led_status[i]));
	    rt_kprintf("i=%d\n", i);
	}
    }
    tid1 = rt_thread_create("t1",
			    thread_entry1,
			    (void*)1,
			    512,
			    25,
			    1);

    if (tid1 != RT_NULL)
	rt_thread_startup(tid1);

    tid2 = rt_thread_create("t2",
			    thread_entry2,
			    (void*)2,
			    512,
			    25,
			    1);

    if (tid2 != RT_NULL)
	rt_thread_startup(tid2);  /* user app entry */

    tid3 = rt_thread_create("t3", 
			    thread_entry3,
			    (void*)3,
			    512,
			    20,
			    1);
    if (tid3 != RT_NULL)
	rt_thread_startup(tid3);  /* user app entry */
    return 0;
}
