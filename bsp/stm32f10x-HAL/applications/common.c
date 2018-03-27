#include <rtthread.h>
#include "common.h"

#define PRSC 71
TIM_HandleTypeDef htim3;

static rt_uint32_t count = 0;

void rt_hw_us_delay(rt_uint16_t us)
{
    rt_uint32_t delta;
    rt_uint16_t maxrange = 1000000/RT_TICK_PER_SECOND;
    if ( us > maxrange ) {
	us = maxrange;
    }
/* 获得延时经过的tick数 */
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
/* 获得当前时间 */
    delta = SysTick->VAL;
/* 循环获得当前时间，直到达到指定的时间后退出循环 */
    while (delta - SysTick->VAL< us);
}

void TIM3_Init(void)
{
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = PRSC - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1000;
    htim3.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子

    HAL_TIM_Base_Init(&htim3);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM3)
    {
	__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
	HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
	HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断
    }
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim3.Instance)
    {
	count++;
    }
}

void reset_distance(void)
{
    count = 0;
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    HAL_TIM_Base_Start_IT(&htim3);
}

rt_uint32_t get_distance(void)
{

    HAL_TIM_Base_Stop_IT(&htim3);
    return 0.17 * (count * 1000 + htim3.Instance->CNT);
}
