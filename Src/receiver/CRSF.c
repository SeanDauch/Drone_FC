#include "stm32f4xx.h"
#include "uart.h"
#include "CRSF.h"

#define sync_byte 0xC8
#define rc_type_byte 0x16
#define rc_frame_len 26
#define rc_data_len 22

#define num_channels 16
#define channel_size 11

#define Min(a,b) (a < b) ? a : b

enum init_status{
    waiting_for_start,
    init_complete
};

void CRSF_init(uint64_t baudrate, uint64_t SystemClock){

    uart1_DMA_init(baudrate, SystemClock);

    enum init_status current_status = waiting_for_start;
    uint8_t reception_data[rc_frame_len] = {0};

    // loop until clear data is found
    while(current_status == waiting_for_start){
        uart1_recieve(reception_data, rc_frame_len);

        for(int i = 0; i<(rc_frame_len - 2); i++){
            if((reception_data[i] == sync_byte) & (reception_data[i+2] == rc_type_byte)){
                current_status = init_complete;
            }
        }
    }
}

void _receive_CRSF_data(uint8_t* data_buffer, uint8_t buffer_length){

    uint8_t temp_byte = 0;
    while(temp_byte != sync_byte){
        uart1_recieve(&temp_byte, 1);
    }

    uint8_t frame_len = 0;
    uart1_recieve(&frame_len, 1);

    // avoid bad data or array overrun
    if(buffer_length < frame_len){
        while(1){}
    }

    uart1_DMA_RX(data_buffer, frame_len);
}

// only input RC data (eg: no data type or CRC)
RC_Data _unpack_rc_CRSF_data(uint8_t* raw_RC_data, uint8_t buffer_length){

    if(buffer_length != rc_data_len){
        while(1){}
    }

    #define uint11_max 0x7FF
    RC_Data return_data = { 
        ((raw_RC_data[0]>>0)  | (raw_RC_data[1]<<8))                          & uint11_max, // 1
        ((raw_RC_data[1]>>3)  | (raw_RC_data[2]<<5))                          & uint11_max, // 2
        ((raw_RC_data[2]>>6)  | (raw_RC_data[3]<<2)  | (raw_RC_data[4]<<10))  & uint11_max, // 3
        ((raw_RC_data[4]>>1)  | (raw_RC_data[5]<<7))                          & uint11_max, // 4
        ((raw_RC_data[5]>>4)  | (raw_RC_data[6]<<4))                          & uint11_max, // 5
        ((raw_RC_data[6]>>7)  | (raw_RC_data[7]<<1)  | (raw_RC_data[8]<<9))   & uint11_max, // 6
        ((raw_RC_data[8]>>2)  | (raw_RC_data[9]<<6))                          & uint11_max, // 7
        ((raw_RC_data[9]>>5)  | (raw_RC_data[10]<<3))                         & uint11_max, // 8
        ((raw_RC_data[11]>>0) | (raw_RC_data[12]<<8))                         & uint11_max, // 9
        ((raw_RC_data[12]>>3) | (raw_RC_data[13]<<5))                         & uint11_max, // 10
        ((raw_RC_data[13]>>6) | (raw_RC_data[14]<<2) | (raw_RC_data[15]<<10)) & uint11_max, // 11
        ((raw_RC_data[15]>>1) | (raw_RC_data[16]<<7))                         & uint11_max, // 12
        ((raw_RC_data[16]>>4) | (raw_RC_data[17]<<4))                         & uint11_max, // 13
        ((raw_RC_data[17]>>7) | (raw_RC_data[18]<<1) | (raw_RC_data[19]<<9))  & uint11_max, // 14
        ((raw_RC_data[19]>>2) | (raw_RC_data[20]<<6))                         & uint11_max, // 15
        ((raw_RC_data[20]>>5) | (raw_RC_data[21]<<3))                         & uint11_max  // 16
    };

    return return_data;
}