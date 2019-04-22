#include "csc.h"
#include "tracer.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

typedef struct bikeResponse
{
    uint8_t flags;
    uint32_t wheelCounter;
    uint16_t lastWheelEvent;
    uint16_t crankCounter;
    uint16_t lastCrankEvent;
} bikeResponse_t;

static bikeResponse_t bikeResponse;
static uint32_t last_timestamp = 0;

void process_csc_data(const uint8_t *data, uint32_t len, Timer & t, val_uint8_t & speed)
{
    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027

    if (len == 11)
    {
        bikeResponse_t r = {0};

        uint8_t p = 0;
        r.flags = data[p++];

        if (r.flags & FLAG_WHEEL_PRESENT)
        {
            r.wheelCounter = data[p++];
            r.wheelCounter |= data[p++] << 8;
            r.wheelCounter |= data[p++] << 16;
            r.wheelCounter |= data[p++] << 24;
            r.lastWheelEvent = data[p++];
            r.lastWheelEvent |= data[p++] << 8;
        }

        if (r.flags & FLAG_CRANK_PRESENT)
        {
            r.crankCounter = data[p++];
            r.crankCounter |= data[p++] << 8;
            r.lastCrankEvent = data[p++];
            r.lastCrankEvent |= data[p++] << 8;
        }

        uint32_t now = t.read_ms();
        uint32_t delta_ms = now - last_timestamp;

        uint32_t crank_counter_diff = r.crankCounter - bikeResponse.crankCounter;
        uint32_t wheel_counter_diff = r.wheelCounter - bikeResponse.wheelCounter;

        FLOW("val: wc=%u, we=%u, cc=%u, ce=%u\r\n", r.wheelCounter, r.lastWheelEvent, r.crankCounter, r.lastCrankEvent);
        FLOW("dif: wc=%u, we=%u, cc=%u, ce=%u\r\n",
             wheel_counter_diff,
             r.lastWheelEvent - bikeResponse.lastWheelEvent,
             crank_counter_diff,
             r.lastCrankEvent - bikeResponse.lastCrankEvent);

        float wheel = 2.170f;

        float crank_per_min = ((float)crank_counter_diff) * 60000.0f / delta_ms;

        float wheel_per_min = (((float)wheel_counter_diff) * 60000.0f / delta_ms);
        float kmh = wheel * wheel_per_min * 60.0f / 1000.0f;
        speed.current = (uint8_t) kmh;

        INFO("now: %ums, diff=%ums, %f kmh, cadence=%f/min\r\n\r\n", now, delta_ms, kmh, wheel_per_min, crank_per_min);

        last_timestamp = now;
        bikeResponse = r;
    }
}
