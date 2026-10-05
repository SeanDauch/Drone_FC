#include "CRSF.h"
#include "RC_receiver.h"
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


#define reciever_length rc_frame_len
int main(){

    swo_init(swo_baud, SystemClock);
    RC_receiver_init(CRSF_baud, SystemClock);
    printf("Initialization Complete\n");
    delay_SysTick(1000, SystemClock);

    RC_controls my_controls = {0};

    uint64_t bad_data_counter = 0;

    while(1){
        while(Receive_RC_controls(&my_controls) == bad_data){bad_data_counter++;}
        print_RC_controls(&my_controls);
        delay_SysTick(500, SystemClock);
    }


    // Debugging the double buffer in DMA
    /*
    CRSF_init(CRSF_baud, SystemClock);

    uint8_t receive_buffer[reciever_length] = {0};

    while (1){
        while(receive_RC_CRSF_data(receive_buffer, reciever_length) == bad_data){}
        RC_Data my_rc = unpack_rc_CRSF_data(&receive_buffer[data_start_pos], rc_data_len);

        printf("CH_1: %d, CH_2: %d, CH_3: %d, CH_4: %d, CH_5: %d\n",
            my_rc.channel_01,
            my_rc.channel_02,
            my_rc.channel_03,
            my_rc.channel_04,
            my_rc.channel_05);
    }


    swo_init(swo_baud, SystemClock);
    uart1_DMA_init(CRSF_baud, SystemClock);
    uint8_t receive_buffer[reciever_length] = {0};
    uint64_t times_through = 0;

    while(1){
        uart1_DMA_RX(receive_buffer, reciever_length);
        delay_SysTick(1000, SystemClock);

        for(int i = 0; i<reciever_length; i++){
            printf("0x%x ", receive_buffer[i]);
        }
        times_through++;
    }


    swo_init(swo_baud, SystemClock);
    uart1_init(CRSF_baud, SystemClock);
    uint8_t crsf_byte =0;
    uint64_t times_through = 0;

    while(1){
        uart1_recieve(&crsf_byte, 1);
        printf("0x%x ", crsf_byte);
        times_through++;
    }*/

    return 1;
}
