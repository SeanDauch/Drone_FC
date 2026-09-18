#ifndef uart_h
    #define uart_h

    #include <stdint.h>

    void uart1_init(uint32_t baud_rate, uint32_t APB2_clk);
    void uart1_transmit(uint8_t data[], uint64_t data_amount);
    void uart1_recieve(uint8_t data_storage[], uint64_t data_amount);

    void uart1_DMA_init(uint32_t baud_rate, uint32_t APB2_clk);
    void uart1_DMA_TX(uint8_t transmition_data[], uint16_t transmition_length);
    void uart1_DMA_RX(uint8_t reception_buffer[], uint16_t buffer_length);
    void uart1_DMA_test(uint64_t CLK_freq);

#endif