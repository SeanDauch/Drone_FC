#ifndef delay_h
    #define delay_h

    #include <stdint.h>

    void delay_SysTick(uint32_t delay_ms, uint32_t System_Frequency);
#endif