#ifndef CRSF_h
    #define CRSF_h
    #include <stdint.h>

    #define sync_byte 0xC8
    #define rc_type_byte 0x16
    #define rc_frame_len 26
    #define rc_data_len 22

    #define type_byte_pos 2
    #define data_start_pos 3
    #define CRC_byte_pos 25

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

    typedef enum data_validity{
        bad_data,
        good_data
    }data_validity;

    data_validity receive_RC_CRSF_data(uint8_t* data_buffer, uint8_t buffer_length);
    RC_Data unpack_rc_CRSF_data(uint8_t* raw_RC_data, uint8_t buffer_length);
    uint8_t generate_crc(const uint8_t * ptr, uint8_t len);

#endif