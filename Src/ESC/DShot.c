#include "delay.h"
#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "pwm.h"

#define DShot_throttle_bitlen 11
#define DShot_telemetry_bitlen 1
#define DShot_checksum_bitlen 4
#define DShot_frame_bitlen 16
#define DShot_timeout 2
#define DShot_numBits DShot_frame_bitlen+DShot_timeout

#define TIM1_CH1_stream 1
#define TIM1_CH1_channel 6
#define TIM1_CH2_stream 2
#define TIM1_CH2_channel 6
#define TIM1_CH3_stream 6
#define TIM1_CH3_channel 6
#define TIM1_CH4_stream 4
#define TIM1_CH4_channel 6

// configuring the DMA2_stream for Dshot
void _DMA2_init(uint8_t stream, uint8_t channel, volatile uint32_t* peripheral_addr){

    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA_Stream_TypeDef*)(DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    DMA2_Streamx->PAR = (uint32_t)peripheral_addr;

    DMA2_Streamx->CR |= (channel << DMA_SxCR_CHSEL_Pos);

    //DMA2_Streamx->CR |= (/*Priority level*/ << DMA_SxCR_PL_Pos);

    DMA2_Streamx->CR |= DMA_SxCR_DIR_0; // memory to peripheral
    DMA2_Streamx->CR |= DMA_SxCR_MINC; // enable memory increment
    DMA2_Streamx->CR |= DMA_SxCR_MSIZE_0; // 16-bit mem size
    DMA2_Streamx->CR &= ~(DMA_SxCR_PINC); // disable peripheral increment
    DMA2_Streamx->CR |= DMA_SxCR_PSIZE_0; // 16-bit periph size
}

// clears all interupt flags for given stream
void _DMA2_clear_flags(uint8_t stream){

    switch(stream){
        case 0: DMA2->LIFCR |= (0x3D<<DMA_LIFCR_CFEIF0_Pos); break;
        case 1: DMA2->LIFCR |= (0x3D<<DMA_LIFCR_CFEIF1_Pos); break;
        case 2: DMA2->LIFCR |= (0x3D<<DMA_LIFCR_CFEIF2_Pos); break;
        case 3: DMA2->LIFCR |= (0x3D<<DMA_LIFCR_CFEIF3_Pos); break;
        case 4: DMA2->HIFCR |= (0x3D<<DMA_HIFCR_CFEIF4_Pos); break;
        case 5: DMA2->HIFCR |= (0x3D<<DMA_HIFCR_CFEIF5_Pos); break;
        case 6: DMA2->HIFCR |= (0x3D<<DMA_HIFCR_CFEIF6_Pos); break;
        case 7: DMA2->HIFCR |= (0x3D<<DMA_HIFCR_CFEIF7_Pos); break;
        default:break;
    }
}

// writes data from memory to Peripheral
void _DMA2_Mem_to_Periph(uint8_t stream, uint16_t* memory_addr, uint16_t num_data_items){

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA_Stream_TypeDef*)(DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    _DMA2_clear_flags(stream);

    DMA2_Streamx->M0AR = (uint32_t)memory_addr;
    DMA2_Streamx->NDTR &= ~(0xFFFF);
    DMA2_Streamx->NDTR |= num_data_items;

    DMA2_Streamx->CR |= DMA_SxCR_EN; // enable DMA
}

uint16_t CCR_1_value = 0;
uint16_t CCR_0_value = 0;

// uses TIM1 and PA8-11
void DShot_init(uint64_t bitrate, uint64_t System_Frequency){

    uint16_t Arr_value = System_Frequency / bitrate;
    CCR_0_value = Arr_value * 0.375f;
    CCR_1_value = 2 * CCR_0_value;

    pwm_init(Arr_value);

    // enable TIM1 DMA requests
    TIM1->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE | TIM_DIER_CC4DE;

    _DMA2_init(TIM1_CH1_stream, TIM1_CH1_channel, &TIM1->CCR1);
    _DMA2_init(TIM1_CH2_stream, TIM1_CH2_channel, &TIM1->CCR2);
    _DMA2_init(TIM1_CH3_stream, TIM1_CH3_channel, &TIM1->CCR3);
    _DMA2_init(TIM1_CH4_stream, TIM1_CH4_channel, &TIM1->CCR4);
}

// packs 11-bit throttle data into DMA ready array
void _pack_data(uint16_t data, uint16_t packed_data_arr[DShot_numBits]){

    // add telemetry to data
    data = data << (DShot_telemetry_bitlen);

    // from Betaflight API
    uint8_t checksum = (data ^ (data >> 4) ^ (data >> 8)) & 0x0F;

    uint16_t bit_frame = 0;
    bit_frame |= (data << DShot_checksum_bitlen) | (checksum << 0);

    // pack the 16 bitframe into an arr for CCR values
    for(uint8_t i = 0; i < DShot_frame_bitlen; i++){

        if((bit_frame & (1 << (DShot_frame_bitlen - 1U - i))) == 0){
            packed_data_arr[i] = CCR_0_value;

        }else{
            packed_data_arr[i] = CCR_1_value;
        }
    }

    // add timeout
    for(uint8_t i = DShot_frame_bitlen; i< DShot_numBits; i++){
        packed_data_arr[i] = 0;
    }
}
 
// Uses PA8
uint16_t CH1_DMA_data_array[DShot_numBits];
void Dshot_write_CH1(uint16_t data){

    // EN turns off at end of transmition
    while(DMA2_Stream1->CR & DMA_SxCR_EN){}

    _pack_data(data, CH1_DMA_data_array);
    _DMA2_Mem_to_Periph(TIM1_CH1_stream, CH1_DMA_data_array,  DShot_numBits);
}

// Uses PA9
uint16_t CH2_DMA_data_array[DShot_numBits];
void Dshot_write_CH2(uint16_t data){

    // EN turns off at end of transmition
    while(DMA2_Stream2->CR & DMA_SxCR_EN){}

    _pack_data(data, CH2_DMA_data_array);
    _DMA2_Mem_to_Periph(TIM1_CH2_stream, CH2_DMA_data_array,  DShot_numBits);
}
 
// Uses PA10
uint16_t CH3_DMA_data_array[DShot_numBits];
void Dshot_write_CH3(uint16_t data){

    // EN turns off at end of transmition
    while(DMA2_Stream6->CR & DMA_SxCR_EN){}

    _pack_data(data, CH3_DMA_data_array);
    _DMA2_Mem_to_Periph(TIM1_CH3_stream, CH3_DMA_data_array,  DShot_numBits);
}

// Uses PA11
uint16_t CH4_DMA_data_array[DShot_numBits];
void Dshot_write_CH4(uint16_t data){

    // EN turns off at end of transmition
    while(DMA2_Stream4->CR & DMA_SxCR_EN){}

    _pack_data(data, CH4_DMA_data_array);
    _DMA2_Mem_to_Periph(TIM1_CH4_stream, CH4_DMA_data_array,  DShot_numBits);
}