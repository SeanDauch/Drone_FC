#ifndef debugging_h
    #define debugging_h

    #include<stdint.h>

    void swo_init(uint64_t swo_freq, uint64_t System_Freq);

    int _write(int file, char *ptr, int len);

    // just use printf
    void print_char_swo(char input_char);
    void print_string_swo(char* string);
#endif