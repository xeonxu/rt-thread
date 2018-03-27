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
#include "common.h"

static rt_sem_t sem = RT_NULL;
static rt_device_t device;
//rt_err_t result = RT_EOK;

#define  SYS_LED     39
#define  SONIC_TGR   17
#define  SONIC_ECHO  26

enum {
    N_SYS_LED,
    N_SONIC_TGR,
    N_SONIC_ECHO,
};

void sonic_hdr(void *args);


struct rt_device_pin_mode gpio_cfg[] =
{
    /* {.pin = 41, .mode = PIN_MODE_OUTPUT}, */
    /* {.pin = 54, .mode = PIN_MODE_OUTPUT}, */
    [N_SYS_LED]         = {.pin = SYS_LED,    .mode = PIN_MODE_OUTPUT},
    [N_SONIC_TGR]       = {.pin = SONIC_TGR,  .mode = PIN_MODE_OUTPUT},
    [N_SONIC_ECHO]      = {.pin = SONIC_ECHO, .mode = PIN_MODE_INPUT_PULLUP},
};

struct rt_device_pin_status gpio_status[] =
{
    /* {.pin = 41, .status = PIN_LOW}, */
    /* {.pin = 54, .status = PIN_HIGH}, */
    [N_SYS_LED]         = {.pin = SYS_LED,    .status = PIN_HIGH},
    [N_SONIC_TGR]       = {.pin = SONIC_TGR,  .status = PIN_LOW},
    [N_SONIC_ECHO]      = {.pin = SONIC_ECHO, .status = PIN_HIGH},
};

struct rt_pin_irq_hdr gpio_irq[] =
{
    [N_SONIC_ECHO]      = {.pin = SONIC_ECHO, .mode = PIN_IRQ_MODE_RISING_FALLING, sonic_hdr, RT_NULL},
};

static void thread_ledblink(void* parameter)
{
    rt_uint32_t i;
    while (1)
    {
        gpio_status[N_SYS_LED].status = !gpio_status[N_SYS_LED].status;
        rt_device_write(device, 0, &gpio_status[N_SYS_LED], sizeof(gpio_status[N_SYS_LED]));
        rt_thread_delay((rt_uint32_t)parameter);
    }
}

void sonic_hdr(void *args)
{
    static rt_uint32_t TIM_SONIC = 0;
    if (PIN_HIGH == rt_pin_read(gpio_irq[N_SONIC_ECHO].pin)) {
	reset_distance();
    }
    else {
        rt_kprintf("dis: %d mm\r\n", get_distance());
    }
}

static void thread_ultrasonic(void* parameter)
{
    rt_uint8_t ch;

    gpio_status[N_SONIC_TGR].status = PIN_LOW;
    rt_device_write(device, 0, &gpio_status[N_SONIC_TGR], sizeof(gpio_status[N_SONIC_TGR]));

    rt_pin_attach_irq(gpio_irq[N_SONIC_ECHO].pin, gpio_irq[N_SONIC_ECHO].mode, gpio_irq[N_SONIC_ECHO].hdr, gpio_irq[N_SONIC_ECHO].args);
    rt_pin_irq_enable(gpio_irq[N_SONIC_ECHO].pin, 1);

    TIM3_Init();

    while (1)
    {
        rt_sem_take(sem, RT_WAITING_FOREVER);
        /* Trigger ultrasonic */
        gpio_status[N_SONIC_TGR].status = PIN_HIGH;
        rt_device_write(device, 0, &gpio_status[N_SONIC_TGR], sizeof(gpio_status[N_SONIC_TGR]));

        rt_hw_us_delay(10);

        gpio_status[N_SONIC_TGR].status = PIN_LOW;
        rt_device_write(device, 0, &gpio_status[N_SONIC_TGR], sizeof(gpio_status[N_SONIC_TGR]));

        rt_sem_release(sem);
        rt_thread_delay((rt_uint32_t)parameter);
    }
}

int main(void)
{
    rt_thread_t tid_ultrasonic;
    rt_thread_t tid_led;
    rt_uint32_t led_delay = 10;
    rt_uint32_t ultrasonic_delay = 10;

    rt_uint8_t i;

    /* Init semphore */
    sem = rt_sem_create("sem", 1, RT_IPC_FLAG_FIFO);
    if (sem == RT_NULL)
    {
        rt_kprintf("create semaphore failed\n");
        return -1;
    }

    /* Init Pin */
    device = rt_device_find("pin");
    if (RT_NULL != device) {
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
        for (i = 0; i < (sizeof(gpio_cfg)/sizeof(gpio_cfg[0])); ++i) {
            rt_device_control(device, 0, &gpio_cfg[i]);
            rt_device_write(device, 0, &gpio_status[i], sizeof(gpio_status[i]));
        }
    }

    tid_led = rt_thread_create("ledblink",
                            thread_ledblink,
                            (void*)led_delay,
                            256,
                            25,
                            1);

    if (tid_led != RT_NULL)
        rt_thread_startup(tid_led); /* led blink */

    tid_ultrasonic = rt_thread_create("ultrasonic",
                            thread_ultrasonic,
                            (void*)ultrasonic_delay,
                            256,
                            2,
                            1);

    if (tid_ultrasonic != RT_NULL)
        rt_thread_startup(tid_ultrasonic);

    return 0;
}
