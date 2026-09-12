#include "stm32f4xx.h"
#include <stdint.h>

#include "ESC.h"
#include "delay.h"

#define System_CLK_Freq 16000000

void enable_FPU(){
    SCB->CPACR |= 0xF<<20;
}

int main(){

    enable_FPU();

    delay_SysTick(1000, System_CLK_Freq);

    ESC_init(300000, System_CLK_Freq);

	while(1){

        // little test for the motors
        for(int i = 0; i < 2000; i++){
            for(int _ = 0; _ < 10; _++){
                set_throttle(1, i);
                delay_SysTick(1, System_CLK_Freq);
            }
        }
        for(int i = 2000; i>=0; i-=250){
            for(int _ = 0; _ < 20; _++){
                set_throttle(1, i);
                delay_SysTick(1, System_CLK_Freq);
            }
        }
    }
}
