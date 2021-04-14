#ifndef __TIMER_H
#define __TIMER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct
{
  // uint32_t start;
  uint32_t totalTickMs;
} timer_drv_t;

void Timer_Init(void);

void System_DelayMs(uint32_t ms);
void Timer_Create(timer_drv_t* t, uint32_t intervalMs);
uint32_t Timer_Timeout(timer_drv_t* t);

#ifdef __cplusplus
}
#endif 

#endif /*__TIMER_H*/