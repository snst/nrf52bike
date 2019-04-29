#ifndef TFTEMU_H
#define TFTEMU_H

#include "mbed.h"
#include "Adafruit_ST7735.h"

class TftEmu : public Adafruit_ST7735 {

public:
TftEmu() : Adafruit_ST7735(1,2,3,4,5,6) {}

void drawBuffer(int16_t x, int16_t y, uint8_t* buf, int16_t w)
{
    uint16_t* p = (uint16_t*) buf;
    while(w--)
    {
        printf("%c", *p++ ? '#' : '.');
    }
    printf("\n");
}

};


#endif