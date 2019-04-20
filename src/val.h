#ifndef VAL_H
#define VAL_H

#include "mbed.h"

typedef struct val_uint8 {
    uint8_t current;
    uint8_t shown;
} val_uint8_t;


static bool new_val(val_uint8_t & v) {
    if(v.current != v.shown) {
        v.shown = v.current;
        return true;
    }
    return false;
}


#endif