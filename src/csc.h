#ifndef CSC_H
#define CSC_H

#include "mbed.h"
#include "val.h"

void process_csc_data(const uint8_t *data, uint32_t len, Timer & t, val_uint8_t & speed);

#endif