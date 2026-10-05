#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include <stdint.h>

// port b
#define TX_pin 6
#define RX_pin 7 

void _enable_FPU(){
    SCB -> CPACR |= 0xF<<20;
}
uint32_t _round_to_int(float input){
    uint32_t output = input;

    if((input - output) >= 0.5f){
        output ++;
    }

    return output;
}

// -------------------------------- NO DMA -------------------------------------

// initializes PB7 as RX and PB6 as TX
void uart1_init(uint32_t baud_rate, uint32_t APB2_clk){

    // -------------- GPIO ----------------
    RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN; 

    GPIOB -> MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOB -> MODER |= GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;

    GPIOB -> OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
    GPIOB -> OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR7_1;

    GPIOB -> AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
    GPIOB -> AFR[0] |= (7<<GPIO_AFRL_AFSEL6_Pos) | (7<<GPIO_AFRL_AFSEL7_Pos);

    // -------------- USART ----------------
    RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

    USART1 -> CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_TE; // UART enable
    USART1 -> CR1 &= ~ USART_CR1_M; // 8 data bits
    USART1 -> CR2 &= ~ USART_CR2_STOP_Msk; // One stop bit


    // baud rate 
    USART1 -> CR1 &= ~(USART_CR1_OVER8);
    _enable_FPU();

    float USARTDIV = (float)APB2_clk/(16 * baud_rate);
    uint16_t DIV_Mantissa = USARTDIV;
    float DIV_Fraction = 16*(USARTDIV - DIV_Mantissa);
    uint8_t rounded_DIV_Fraction = _round_to_int(DIV_Fraction);

    USART1 -> BRR &= ~(USART_BRR_DIV_Mantissa_Msk | USART_BRR_DIV_Fraction_Msk);
    USART1 -> BRR |= (DIV_Mantissa << USART_BRR_DIV_Mantissa_Pos)|(rounded_DIV_Fraction << USART_BRR_DIV_Fraction_Pos);
}

void uart1_transmit(uint8_t data[], uint64_t data_amount){

    for(uint64_t i = 0; i<data_amount; i++){

        while(!(USART1->SR & USART_SR_TXE)){} // wait for transmitter to empty

        USART1->DR = data[i];
    }

    while(!(USART1->SR & USART_SR_TC)){} // wait for transmition to complete
}

void uart1_recieve(uint8_t data_storage[], uint64_t data_amount){

    for(uint64_t i = 0; i<data_amount; i++){

        while(!(USART1->SR & USART_SR_RXNE)){} // wait for data to get received

        data_storage[i] = USART1 -> DR;
    }
}

void _clear_USART1_SR(){

    uint32_t temp_read;
    temp_read = USART1 -> SR;
    temp_read = USART1 -> DR;
}

// ----------------------------------- DMA -------------------------------------
#define USART_TX_stream 7
#define USART_TX_channel 4
#define USART_RX_stream 5
#define USART_RX_channel 4

#define DMA_DIR_Periph_to_Mem 0
#define DMA_DIR_Mem_to_Periph 1
#define DMA_DIR_Mem_to_Mem 2

// configuring the DMA2_stream for UART
void _DMA2_init_for_USART(uint8_t stream, uint8_t channel, volatile uint32_t* peripheral_addr, uint8_t DIR_bits){

    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA_Stream_TypeDef*)(DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    DMA2_Streamx->PAR = (uint32_t)peripheral_addr;

    DMA2_Streamx->CR |= (channel << DMA_SxCR_CHSEL_Pos);

    //DMA2_Streamx->CR |= (/*Priority level*/ << DMA_SxCR_PL_Pos);

    DMA2_Streamx->CR &= ~DMA_SxCR_DIR;
    DMA2_Streamx->CR |= (DIR_bits<<DMA_SxCR_DIR_Pos);

    DMA2_Streamx->CR |= DMA_SxCR_MINC; // enable memory increment
    DMA2_Streamx->CR &= ~DMA_SxCR_MSIZE; // 8-bit mem size
    DMA2_Streamx->CR &= ~(DMA_SxCR_PINC); // disable peripheral increment
    DMA2_Streamx->CR &= ~ DMA_SxCR_PSIZE_0; // 8-bit periph size
}

// clears all interupt flags for given stream
void _DMA2_clear_flags_for_USART(uint8_t stream){

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

// set memory address and enable DMA stream
void _DMA2_enable(uint8_t stream, uint8_t* memory_addr, uint16_t num_data_items){

    DMA_Stream_TypeDef* DMA2_Streamx = (DMA_Stream_TypeDef*)(DMA2_Stream0_BASE + (0x18 * stream));

    DMA2_Streamx->CR &= ~(DMA_SxCR_EN);
    while(DMA2_Streamx->CR & DMA_SxCR_EN){} // wait for stream to disable

    _DMA2_clear_flags_for_USART(stream);

    DMA2_Streamx->M0AR = (uint32_t)memory_addr;

    DMA2_Streamx->NDTR &= ~(0xFFFF);
    DMA2_Streamx->NDTR |= num_data_items;

    DMA2_Streamx->CR |= DMA_SxCR_EN; // enable DMA
}

// uses DMA2 stream 5 and 7
void uart1_DMA_init(uint32_t baud_rate, uint32_t APB2_clk){

    uart1_init(baud_rate, APB2_clk);

    _DMA2_init_for_USART(USART_TX_stream, USART_TX_channel, &USART1->DR, DMA_DIR_Mem_to_Periph);
    _DMA2_init_for_USART(USART_RX_stream, USART_RX_channel, &USART1->DR, DMA_DIR_Periph_to_Mem); 

    USART1->CR3 |= USART_CR3_DMAR|USART_CR3_DMAT;

    
}

uint64_t heartbeat = 0;
// pin b6
void uart1_DMA_TX(uint8_t transmition_data[], uint16_t transmition_length){

    // stream disables on TC
    while(DMA2_Stream7 -> CR & DMA_SxCR_EN){}

    heartbeat++;

    _clear_USART1_SR();
    _DMA2_enable(USART_TX_stream, transmition_data, transmition_length);
}

// pin b7
void uart1_DMA_RX(uint8_t reception_buffer[], uint16_t buffer_length){

    // stream disables on TC
    while(DMA2_Stream5 -> CR & DMA_SxCR_EN){}

    _clear_USART1_SR();
    _DMA2_enable(USART_RX_stream, reception_buffer, buffer_length);
}

// connect tx to rx and set breakpoint at top of while loop
void uart1_DMA_test(uint64_t CLK_freq){

    uart1_DMA_init(9600, CLK_freq);
    uint8_t Transmition_data[] = {0,1,2,3,4,5};
    uint8_t Reception_data[6] = {0};

    while(1){
        uart1_DMA_RX(Reception_data, 6);
        uart1_DMA_TX(Transmition_data, 6);

        for(volatile int i = 0; i<100000; i++){} //wait a sec

        for(int i = 0; i<6; i++){
            Transmition_data[i] *= 2;
            Transmition_data[i] ++;
        }
    }
}

