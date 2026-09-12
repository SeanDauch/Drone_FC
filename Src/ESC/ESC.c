#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "ESC.h"
#include "DShot.h"
#include "delay.h"

#define min_throttle 48
#define max_throttle 2047

void ESC_init(uint64_t bitrate, uint64_t System_Frequency){

    // wait for ESC power up
    delay_SysTick(2000, System_Frequency);
    DShot_init(bitrate, System_Frequency);

    // send arbitrary throttle for 1 second
    uint64_t transmitions_per_sec = bitrate/DShot_numBits;
    for(uint64_t i = 0; i<(transmitions_per_sec); i++){
        set_throttle(1, 100);
        set_throttle(2, 100);
        set_throttle(3, 100);
        set_throttle(4, 100);
    }
    
    // send 0% throttle for 1 second
    for(uint64_t i = 0; i<(transmitions_per_sec); i++){
        set_throttle(1, 0);
        set_throttle(2, 0);
        set_throttle(3, 0);
        set_throttle(4, 0);
    }
}

// throttle 0-1999
void set_throttle(uint8_t motor_number, uint16_t throttle){

    throttle += min_throttle;

    if(throttle < min_throttle){
        throttle = min_throttle;
    }else if(throttle > max_throttle){
        throttle = max_throttle;
    }
    
    switch (motor_number){
        case 1: Dshot_write_CH1(throttle); break;
        case 2: Dshot_write_CH2(throttle); break;
        case 3: Dshot_write_CH3(throttle); break;
        case 4: Dshot_write_CH4(throttle); break;
        default:break;
    }
}