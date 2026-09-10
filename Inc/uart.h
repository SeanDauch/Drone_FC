#ifndef uart_h
    #define uart_h

    #include <stdint.h>

    void uart1_init(uint32_t baud_rate, uint32_t APB2_clk);
    void uart1_transmit(uint8_t data[], uint64_t data_amount);
    void uart1_recieve(uint8_t data_storage[], uint64_t data_amount);

#endif