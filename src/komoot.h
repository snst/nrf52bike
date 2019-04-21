#ifndef KOMOOT_H
#define KOMOOT_H

#include <mbed_events.h>
#include <rtos.h>
#include "mbed.h"
#include "ble/BLE.h"
#include <sys/time.h>
#include "val.h"

void process_komoot_data(const uint8_t *data, uint32_t len, val_uint8_t &dir, val_uint32_t &dist, uint8_t *street, uint8_t street_len);

#endif