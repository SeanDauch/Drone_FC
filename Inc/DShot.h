#ifndef Dshot_h
    #define Dshot_h
    #include <stdint.h>

    #define DShot_frame_bitlen 16
    #define DShot_timeout 2
    #define DShot_numBits DShot_frame_bitlen+DShot_timeout

    void DShot_init(uint64_t bitrate, uint64_t System_Frequency);
    
    void Dshot_write_CH1(uint16_t data);
    void Dshot_write_CH2(uint16_t data);
    void Dshot_write_CH3(uint16_t data);
    void Dshot_write_CH4(uint16_t data);
#endif