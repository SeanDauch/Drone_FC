#include "stm32f4xx.h"
#include "delay.h"

void delay_SysTick(uint32_t delay_ms, uint32_t System_Frequency){

    SysTick->LOAD |= (System_Frequency/1000u) - 1u;

    SysTick->VAL = 0;

    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    for(uint32_t i = 0; i < delay_ms; i++){
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)){}
    }

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}