#ifndef MBED_H
#define MBED_H

#include <cstddef>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int32_t PinName; 
typedef int32_t DigitalOut;


class SPI {
    public:
    SPI(PinName a, PinName b, PinName c) {}
    void write(uint8_t c) {}
    void format(uint32_t a, uint32_t b) {}
    void frequency(uint32_t a) {}
    void write(const char* a, int b, void* c, int d) {}
};

class Stream {
    public:
};

class Timer
{
    public:
    uint32_t read_ms() { return 5; }
};

static void wait_ms(uint32_t ms) {}


#define TFT_MOSI 1
#define TFT_MISO 2
#define TFT_SCLK 3
#define TFT_CS 4
#define TFT_DC 5
#define TFT_RST 6


#endif