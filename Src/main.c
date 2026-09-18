#include "stm32f4xx.h"
#include <stdint.h>

#include "uart.h"

#include "ESC.h"
#include "delay.h"

#define System_CLK_Freq 16000000

void enable_FPU(){
    SCB->CPACR |= 0xF<<20;
}

int main(){

    uart1_DMA_test(System_CLK_Freq);
}
