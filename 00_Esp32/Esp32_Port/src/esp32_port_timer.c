#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <logfile.h>

#include "timer_drv.h"

static uint32_t count;
static timer_t gTimerid;

void SysTick_Handler_Fun(void);

void timer_callback(int sig) 
{
    SysTick_Handler_Fun();
}

void Timer_Init(void)
{
    LOGI("Init timer");
    struct itimerspec value;
    
    // Setup input
    value.it_value.tv_sec = 1;
    value.it_value.tv_nsec = 0;
    value.it_interval.tv_sec = 1;
    value.it_interval.tv_nsec = 0;
    
    timer_create (CLOCK_REALTIME, NULL, &gTimerid);
    timer_settime (gTimerid, 0, &value, NULL);

    (void) signal(SIGALRM, timer_callback);

}

void SysTick_Handler_Fun(void)
{
    count++;
}

void System_DelayMs(uint32_t ms)
{
    usleep(1000);
}

void Timer_Create(timer_drv_t* t, uint32_t intervalMs)
{
    t->totalTickMs = count + intervalMs;
}

uint32_t Timer_Timeout(timer_drv_t* t)
{
    uint32_t r=((count > t->totalTickMs) ? 1 : 0);
    return r;
}

