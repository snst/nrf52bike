#include "AppCsc.h"
#include "IDataCsc.h"
#include "tracer.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

AppCsc::AppCsc(IDataCsc *csc)
    : is_init_(false), csc_sink_(csc), wheel_size_cm_(217.0f)
{
    memset(&csc_data_, 0, sizeof(csc_data_));
    memset(&filtered_speed_kmhX10_, 0, sizeof(filtered_speed_kmhX10_));
}

AppCsc::~AppCsc()
{
}

void AppCsc::UpdateGUI()
{
    INFO("AppCsc::UpdateGUI()\r\n");
    csc_sink_->Update(csc_data_);
}

void AppCsc::ProcessData(uint32_t now_ms, const uint8_t *data, uint32_t len)
{
    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027
    if (len == 11)
    {
        cscResponse_t csc = {0};

        uint8_t p = 0;
        csc.flags = data[p++];

        if (csc.flags & FLAG_WHEEL_PRESENT)
        {
            csc.wheelCounter = data[p++];
            csc.wheelCounter |= data[p++] << 8;
            csc.wheelCounter |= data[p++] << 16;
            csc.wheelCounter |= data[p++] << 24;
            csc.wheelEvent = data[p++];
            csc.wheelEvent |= data[p++] << 8;
        }

        if (csc.flags & FLAG_CRANK_PRESENT)
        {
            csc.crankCounter = data[p++];
            csc.crankCounter |= data[p++] << 8;
            csc.crankEvent = data[p++];
            csc.crankEvent |= data[p++] << 8;
        }

        if (is_init_)
        {
            uint32_t delta_ms = now_ms - csc_data_.timestamp_ms;
            uint32_t wheel_counter_diff = csc.wheelCounter - last_csc_.wheelCounter;
            uint32_t wheel_event_diff = csc.wheelEvent - last_csc_.wheelEvent;
            double delta_sec = wheel_event_diff / 1024.0;
            double speed_kmh = 0.0f;

            csc_data_.is_riding = wheel_counter_diff > 0;
            csc_data_.total_wheel_rounds += wheel_counter_diff;

            if (wheel_event_diff)
            {
                // cm/sec * 3600 / 100 / 1000
                csc_data_.speed_kmhX10 = (uint16_t)((wheel_size_cm_ * wheel_counter_diff * 3.6f) / delta_sec / 10.0f);
                INFO("SPEED %u\r\n", wheel_event_diff);
            }
            else
            {
                csc_data_.speed_kmhX10 = 0;
                INFO("SPEED 0000!! %u\r\n", wheel_event_diff);
            }

            uint32_t crank_counter_diff = csc.crankCounter - last_csc_.crankCounter;
            uint32_t crank_event_diff = csc.crankEvent - last_csc_.crankEvent;
            double crank_delta_sec = crank_event_diff / 1024.0;

            if (crank_delta_sec > 0.0f)
            {
                csc_data_.cadence = crank_counter_diff * 60.0f / crank_delta_sec;
            }

            INFO("wc=%u, we=%u, dwc=%u, dwe=%u, ms=%u, s=%f, r=%u\r\n",
                 csc.wheelCounter, csc.wheelEvent, wheel_counter_diff, wheel_event_diff,
                 delta_ms, delta_sec, csc_data_.total_wheel_rounds);

            if (csc_data_.is_riding)
            {
                csc_data_.time_ms += delta_ms;
            }

            INFO("now: %ums, diff=%ums, %f kmh, cadence=%u/min, time=%u\r\n\r\n", now_ms, delta_ms, csc_data_.speed_kmhX10 / 10.0f, (uint16_t)csc_data_.cadence, csc_data_.time_ms / 1000);

            csc_data_.distance_cm = (uint32_t)(wheel_size_cm_ * csc_data_.total_wheel_rounds);

            CalculateAverageSpeed();
        }

        csc_data_.timestamp_ms = now_ms;
        last_csc_ = csc;
        is_init_ = true;
    }
}

void AppCsc::CalculateAverageSpeed()
{
    if (csc_data_.time_ms > 0.0f)
    {
        // cm / 100 * 3.6
        csc_data_.average_speed_kmhX10 = (uint16_t)(1000.0f * 0.36f * csc_data_.distance_cm / csc_data_.time_ms);
    }

    uint32_t sum = filtered_speed_kmhX10_[0] + csc_data_.speed_kmhX10;
    for(size_t i=1; i<SPEED_FILTER_VALUES_MAX; i++) {
        sum += filtered_speed_kmhX10_[i];
        filtered_speed_kmhX10_[i-1] = filtered_speed_kmhX10_[i];
    }
    filtered_speed_kmhX10_[SPEED_FILTER_VALUES_MAX-1] = csc_data_.speed_kmhX10;
    csc_data_.filtered_speed_kmhX10 = sum / SPEED_FILTER_VALUES_CNT;
}