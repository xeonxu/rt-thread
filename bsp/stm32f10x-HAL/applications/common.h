#ifndef APP_COMMON__
#define APP_COMMON__

#include <rtthread.h>
#include <stm32f1xx_hal.h>
#include <core_cm3.h>

#ifdef __cplusplus
extern "C" {
#endif

extern TIM_HandleTypeDef htim3;
  
extern void rt_hw_us_delay(rt_uint16_t us);
extern rt_uint32_t get_distance(void);
extern void TIM3_Init(void);
extern void reset_distance(void);

#ifdef __cplusplus
}
#endif

#endif	// APP_COMMON__
