#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include "mbed.h"

extern Timer timer;

void ConvertUtf8toAscii(const uint8_t* in, uint16_t in_len, char* out, uint16_t out_len);
void StartTimer();
uint32_t GetMillis();

#endif