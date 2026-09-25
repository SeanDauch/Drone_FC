#include "stm32f4xx.h"
#include "uart.h"

// SWO is PB3
void swo_init(uint64_t swo_freq, uint64_t System_Freq){

    // GPIO config
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER &= ~GPIO_MODER_MODE3;
    GPIOB->MODER |= GPIO_MODER_MODE3_1;

    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3; // fastest speed

    GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL3;

    // TPIU
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;
    DBGMCU->CR &= ~DBGMCU_CR_TRACE_MODE;

    TPI->ACPR &= ~TPI_ACPR_PRESCALER_Msk;
    TPI->ACPR |= (System_Freq/swo_freq)-1;

    TPI->SPPR = 2; // NRZ mode
    TPI->FFCR &= ~TPI_FFCR_EnFCont_Msk; // no formatting because only using SWO

    //TPI->DEVID

    // ITM config
    ITM->LAR =  0xC5ACCE55;
    ITM->TCR |= ITM_TCR_ITMENA_Msk | ITM_TCR_SYNCENA_Msk | (1<<ITM_TCR_TraceBusID_Pos);
    ITM->TER |= 1;
    ITM->TPR |= 1;
}

void _swo_write(uint32_t data){

  while(ITM->TCR & ITM_TCR_BUSY_Msk){}

  ITM->PORT[0].u32 = data;
}

// Requires swo_init
int _write(int file, char *ptr, int len){
  (void)file;

  for(int i = 0; i<len; i++){
    _swo_write((uint32_t)ptr[i]);
  }

  return len;
}

void print_char_swo(char input_char){
  _swo_write(input_char);
}

void print_string_swo(char* string){

  for(int i = 0; string[i] != '\0'; i++){
    _swo_write(string[i]);
  }
}