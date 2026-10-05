#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "uart.h"
#include <string.h>
#include "CRSF.h"

#define sync_byte 0xC8
#define rc_type_byte 0x16
#define rc_frame_len 26
#define rc_data_len 22
#define RX_buffer_len 2*rc_frame_len-1

#define RX_DMA_stream DMA2_Stream5

#define num_channels 16
#define channel_size 11

#define Min(a,b) (a < b) ? a : b
#define Max(a,b) (a > b) ? a : b

// -------------------------- Setup -------------------------------------
void CRSF_init(uint64_t baudrate, uint64_t SystemClock){

    uart1_DMA_init(baudrate, SystemClock);

    //enable double buffer for RX
    static uint8_t recieve_buffer0[RX_buffer_len] = {0};
    static uint8_t recieve_buffer1[RX_buffer_len] = {0};

    RX_DMA_stream->CR |= DMA_SxCR_DBM | DMA_SxCR_CIRC;
    RX_DMA_stream->M0AR = (uint32_t)recieve_buffer0;
    RX_DMA_stream->M1AR = (uint32_t)recieve_buffer1;

    RX_DMA_stream->NDTR &= ~(0xFFFF);
    RX_DMA_stream->NDTR |= RX_buffer_len;

    RX_DMA_stream->CR |= DMA_SxCR_EN;
}

// -------------------------- reception -------------------------------------

// Returns index of sync byte.
// -1 means unable to find valid index.
// Buffer_len must be >= 26
int8_t _find_RC_frame_in_buffer(uint8_t* buffer, uint8_t buffer_len){

    for(int i = buffer_len - rc_frame_len; i>=0; i--){
        if((buffer[i] == sync_byte) & (buffer[i+2] == rc_type_byte)){ // valid frame start
            return i;
        }
    }

    return -1; // couldnt find valid index
}

// data buffer will remain unchanged if no frame is found
data_validity receive_RC_CRSF_data(uint8_t* data_buffer, uint8_t buffer_length){

    uint8_t* buffer_0_addr = (uint8_t*)RX_DMA_stream->M0AR;
    uint8_t* buffer_1_addr = (uint8_t*)RX_DMA_stream->M1AR;

    if((RX_DMA_stream->CR & DMA_SxCR_CT) == 0){

        int8_t frame_start_index = _find_RC_frame_in_buffer(buffer_1_addr, RX_buffer_len);

        if(frame_start_index != -1){
            memcpy(data_buffer, &buffer_1_addr[frame_start_index], rc_frame_len*sizeof(uint8_t));
            return good_data;
        }
        
    }else{

        int8_t frame_start_index = _find_RC_frame_in_buffer(buffer_0_addr, RX_buffer_len);

        if(frame_start_index != -1){
            memcpy(data_buffer, &buffer_0_addr[frame_start_index], rc_frame_len*sizeof(uint8_t));
            return good_data;
        }
    }

    return bad_data;
}

// only input RC data (eg: no data type or CRC)
RC_Data unpack_rc_CRSF_data(uint8_t* raw_RC_data, uint8_t buffer_length){

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

// -------------------------- CRC  -------------------------------------
/// CRC code is directly from CRSF github

uint8_t crc8tab[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

// doesnt include sync byte and frame length
uint8_t generate_crc(const uint8_t * ptr, uint8_t len){
    uint8_t crc = 0;
    for (uint8_t i=0; i<len; i++)
        crc = crc8tab[crc ^ *ptr++];
    return crc;
}