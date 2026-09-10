#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "pwm.h"

#define TIM1_CH1_stream 1
#define TIM1_CH1_channel 6
#define TIM1_CH2_stream 2
#define TIM1_CH2_channel 6
#define TIM1_CH3_stream 6
#define TIM1_CH3_channel 6
#define TIM1_CH4_stream 4
#define TIM1_CH4_channel 6

// configuring the DMA2_stream for Dshot
void _DMA2_init(uint8_t stream, uint8_t channel, uint32_t* peripheral_addr){

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    DMA2_Streamx->PAR = peripheral_addr;

    DMA2_Streamx->CR |= (channel << DMA_SxCR_CHSEL_Pos);

    //DMA2_Streamx->CR |= (/*Priority level*/ << DMA_SxCR_PL_Pos);

    DMA2_Streamx->CR |= DMA_SxCR_DIR_0; // memory to peripheral
    DMA2_Streamx->CR |= DMA_SxCR_MINC; // enable memory increment
    DMA2_Streamx->CR |= DMA_SxCR_MSIZE_0; // 16-bit mem size
    DMA2_Streamx->CR &= ~(DMA_SxCR_PINC); // disable peripheral increment
    DMA2_Streamx->CR |= DMA_SxCR_PSIZE_0; // 16-bit periph size
}

// writes data from memory to Peripheral
void _DMA2_Mem_to_Periph(uint8_t stream, uint32_t* memory_addr, uint16_t num_data_items){

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    DMA2_Streamx->M0AR = memory_addr;
    DMA2_Streamx->NDTR &= ~(0xFFFF);
    DMA2_Streamx->NDTR |= num_data_items;

    DMA2_Streamx->CR |= DMA_SxCR_EN; // enable DMA
}


uint16_t CCR_1_value = 0;
uint16_t CCR_0_value = 0;

void DShot_init(uint64_t bitrate, uint64_t System_Frequency){

    uint16_t Arr_value = System_Frequency / bitrate;
    CCR_0_value = Arr_value * 0.375f;
    CCR_1_value = 2 * CCR_0_value;

    pwm_init(Arr_value);

    _DMA2_init(TIM1_CH1_stream, TIM1_CH1_channel, &TIM1->CCR1);
    _DMA2_init(TIM1_CH2_stream, TIM1_CH2_channel, &TIM1->CCR2);
    _DMA2_init(TIM1_CH3_stream, TIM1_CH3_channel, &TIM1->CCR3);
    _DMA2_init(TIM1_CH4_stream, TIM1_CH4_channel, &TIM1->CCR4);
}