#include "CRSF.h"
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

#include "uart.h"
#include "debugging.h"
#include "CRSF.h"

#include "ESC.h"
#include "delay.h"

#define SystemClock 16000000
#define swo_baud 2000000
#define CRSF_baud 420000

void enable_FPU(){
    SCB->CPACR |= 0xF<<20;
}


#define reciever_length 26
int main(){

    CRSF_init(CRSF_baud, SystemClock);

    uint8_t receive_buffer[24] = {0};

    while (1){
        _receive_CRSF_data(receive_buffer, 24);
        delay_SysTick(1000,SystemClock);
        RC_Data my_rc = _unpack_rc_CRSF_data(&receive_buffer[2], 22);
    }

    return 1;
}
