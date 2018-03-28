/*
 * File      : drv_hwtimer.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author           Notes
 * 2015-09-02     heyuanjie87      the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stm32f1xx_hal.h>
#include <core_cm3.h>
#include "drv_hwtimer.h"

#ifdef RT_USING_HWTIMER

TIM_HandleTypeDef htim;

static void NVIC_Configuration(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();            //使能TIM2时钟
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);    //设置中断优先级，抢占优先级0，子优先级0
    HAL_NVIC_EnableIRQ(TIM2_IRQn);          //开启TIM2中断
}

static void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
    TIM_TypeDef *tim;

    tim = (TIM_TypeDef *)timer->parent.user_data;

    if (state == 1)
    {
	htim.Instance = tim;
	htim.Init.Prescaler = 72 - 1;
	htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim.Init.Period = 1000;
	htim.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子

	HAL_TIM_Base_Init(&htim);
	NVIC_Configuration();
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{
    TIM_TypeDef *tim;

    __HAL_TIM_SET_COUNTER(&htim, 0);
    while(HAL_OK != HAL_TIM_Base_Start_IT(&htim));

    return RT_EOK;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    TIM_TypeDef *tim;

    HAL_TIM_Base_Stop_IT(&htim);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    TIM_TypeDef *tim;
    rt_err_t err = RT_EOK;

    tim = (TIM_TypeDef *)timer->parent.user_data;

    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
    {
        uint16_t val;
        rt_uint32_t freq, sysfreq;

        freq = *((rt_uint32_t*)arg);
        sysfreq = HAL_RCC_GetHCLKFreq();
        val = sysfreq / freq;

	htim.Init.Prescaler = val - 1;
	timer->period_sec = 1000;
	timer->freq = freq;
	TIM_Base_SetConfig(htim.Instance, &htim.Init);
    }
    break;
    default:
    {
        err = -RT_ENOSYS;
    }
    break;
    }

    return err;
}

static rt_uint32_t timer_counter_get(struct rt_hwtimer_device *timer)
{
    return __HAL_TIM_GET_COUNTER(&htim);
}

static const struct rt_hwtimer_info _info =
{
    1000000,           /* the maximum count frequency can be set */
    2000,              /* the minimum count frequency can be set */
    0xFFFF,            /* the maximum counter value */
    HWTIMER_CNTMODE_UP,/* Increment or Decreasing count mode */
};

static const struct rt_hwtimer_ops _ops =
{
    timer_init,
    timer_start,
    timer_stop,
    timer_counter_get,
    timer_ctrl,
};

static rt_hwtimer_t _timer0;

int stm32_hwtimer_init(void)
{
    _timer0.info = &_info;
    _timer0.ops  = &_ops;

    rt_device_hwtimer_register(&_timer0, "timer0", TIM2);

    return 0;
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim);
}

INIT_BOARD_EXPORT(stm32_hwtimer_init);
#endif
