#ifndef CRSF_h
    #define CRSF_h
    #include <stdint.h>

    typedef struct RC_Data{
        uint16_t channel_01: 11;
        uint16_t channel_02: 11;
        uint16_t channel_03: 11;
        uint16_t channel_04: 11;
        uint16_t channel_05: 11;
        uint16_t channel_06: 11;
        uint16_t channel_07: 11;
        uint16_t channel_08: 11;
        uint16_t channel_09: 11;
        uint16_t channel_10: 11;
        uint16_t channel_11: 11;
        uint16_t channel_12: 11;
        uint16_t channel_13: 11;
        uint16_t channel_14: 11;
        uint16_t channel_15: 11;
        uint16_t channel_16: 11;
    } RC_Data;

    void CRSF_init(uint64_t baudrate, uint64_t SystemClock);
    void _receive_CRSF_data(uint8_t* data_buffer, uint8_t buffer_length);
    RC_Data _unpack_rc_CRSF_data(uint8_t* raw_RC_data, uint8_t buffer_length);

#endif