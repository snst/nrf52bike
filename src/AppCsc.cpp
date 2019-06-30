#include "AppCsc.h"
#include "tracer.h"
#include "common.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

#define MAX_SPEED (999u)
#define MAX_DISTANCE (99999999u)
#define MAX_CADENCE (199u)
#define MAX_TIME (((99u * 3600u) + (99u * 60u) + 59u) * 1000u)

AppCsc::AppCsc()
    : is_init_(false), wheel_size_cm_(214.0f), crank_event_sum_(0), crank_counter_sum_(0)
{
    memset(&data_, 0, sizeof(data_));
    memset(&filtered_speed_kmhX10_, 0, sizeof(filtered_speed_kmhX10_));
    memset(&filtered_cadence_, 0, sizeof(filtered_cadence_));
}

AppCsc::~AppCsc()
{
}

uint16_t GetDiffUInt16(uint16_t now, uint16_t last)
{
    uint16_t diff;
    if (now >= last)
    {
        diff = now - last;
    }
    else
    {
        diff = now + (0xFFFF - last);
    }
    return diff;
}

uint32_t GetDiffUInt32(uint32_t now, uint32_t last)
{
    uint32_t diff;
    if (now >= last)
    {
        diff = now - last;
    }
    else
    {
        diff = now + (0xFFFFFFFF - last);
    }
    return diff;
}

bool AppCsc::ProcessData(uint32_t now_ms, const uint8_t *data, uint32_t len)
{
    bool ret = false;
    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027
    if (len == 11u)
    {
        cscMsg_t msg = {0};
        uint8_t p = 0u;
        msg.flags = data[p++];

        if (msg.flags & FLAG_WHEEL_PRESENT)
        {
            msg.wheelCounter = data[p++];
            msg.wheelCounter |= data[p++] << 8u;
            msg.wheelCounter |= data[p++] << 16u;
            msg.wheelCounter |= data[p++] << 24u;
            msg.wheelEvent = data[p++];
            msg.wheelEvent |= data[p++] << 8u;
        }

        if (msg.flags & FLAG_CRANK_PRESENT)
        {
            msg.crankCounter = data[p++];
            msg.crankCounter |= data[p++] << 8u;
            msg.crankEvent = data[p++];
            msg.crankEvent |= data[p++] << 8u;
        }

        if (is_init_)
        {
            uint32_t delta_ms = now_ms - data_.timestamp_ms;
            uint32_t wheel_counter_diff = GetDiffUInt32(msg.wheelCounter, last_msg_.wheelCounter);
            uint32_t wheel_event_diff = GetDiffUInt16(msg.wheelEvent, last_msg_.wheelEvent);
            double delta_sec = wheel_event_diff / 1024.0;
            double speed_kmh = 0.0f;

            FLOW("WE %u %u\r\n", msg.wheelEvent, wheel_event_diff);

            bool is_riding = wheel_counter_diff > 0u;
            data_.is_riding = is_riding;
            data_.total_wheel_rounds += wheel_counter_diff;

            uint16_t speed_kmhX10 = 0u;
            
            if (wheel_event_diff)
            {
                // cm/sec * 3600 / 100 / 1000
                speed_kmhX10 = (uint16_t)((wheel_size_cm_ * wheel_counter_diff * 3.6f) / delta_sec / 10.0f);
            }

            data_.speed_kmhX10 = MIN(speed_kmhX10, MAX_SPEED);

            uint32_t crank_counter_diff = GetDiffUInt16(msg.crankCounter, last_msg_.crankCounter);
            uint32_t crank_event_diff = GetDiffUInt16(msg.crankEvent, last_msg_.crankEvent);
            double crank_delta_sec = crank_event_diff / 1024.0f;

            uint16_t cadence = 0u;
            uint16_t average_cadence = data_.average_cadence;

            if (crank_delta_sec > 0.0f)
            {
                cadence = crank_counter_diff * 60.0f / crank_delta_sec;
                if (cadence > 30u)
                {
                    crank_event_sum_ += crank_event_diff;
                    crank_counter_sum_ += crank_counter_diff;
                    average_cadence = crank_counter_sum_ * 60.0f / ((double)crank_event_sum_ / 1024.0f);
                }
            }

            data_.cadence = MIN(cadence, MAX_CADENCE);
            data_.average_cadence = MIN(average_cadence, MAX_CADENCE);
            uint16_t filtered_cadence = AddFilterVal(filtered_cadence_, data_.cadence);
            data_.filtered_cadence = MIN(filtered_cadence, MAX_CADENCE);

            /*
            INFO("wc=%u, we=%u, dwc=%u, dwe=%u, ms=%u, s=%f, r=%u\r\n",
                 msg.wheelCounter, msg.wheelEvent, wheel_counter_diff, wheel_event_diff,
                 delta_ms, delta_sec, data_.total_wheel_rounds);
*/
            uint32_t trip_time_ms = data_.trip_time_ms;
            if (data_.is_riding)
            {
                trip_time_ms += delta_ms;
            }
            data_.trip_time_ms = MIN(trip_time_ms, MAX_TIME);
            static uint8_t k = 0u;
            FLOW("[%i]now: %ums, diff=%ums, %f kmh, cadence=%u/min, time=%u\r\n", k++, now_ms, delta_ms, data_.speed_kmhX10 / 10.0f, (uint16_t)data_.cadence, data_.trip_time_ms / 1000);

            uint32_t trip_distance_cm = (uint32_t)(data_.total_wheel_rounds * wheel_size_cm_);
            data_.trip_distance_cm = MIN(trip_distance_cm, MAX_DISTANCE);

            CalculateSpeedValues();
        }

        data_.timestamp_ms = now_ms;
        last_msg_ = msg;
        is_init_ = true;
        ret = true;
    }
    return ret;
}

void AppCsc::CalculateSpeedValues()
{
    uint16_t average_speed_kmhX10 = data_.average_speed_kmhX10;
    if (data_.trip_time_ms > 0.0f)
    {
        // cm / 100 * 3.6
        average_speed_kmhX10 = (uint16_t)(1000.0f * 0.36f * data_.trip_distance_cm / data_.trip_time_ms);
    }
    data_.average_speed_kmhX10 = MIN(average_speed_kmhX10, MAX_SPEED);

    uint16_t filtered_speed_kmhX10 = AddFilterVal(filtered_speed_kmhX10_, data_.speed_kmhX10);
    data_.filtered_speed_kmhX10 = MIN(filtered_speed_kmhX10, MAX_SPEED);
    data_.max_speed_kmhX10 = MAX(data_.max_speed_kmhX10, data_.filtered_speed_kmhX10);
}

uint16_t AppCsc::AddFilterVal(uint16_t array[], uint16_t val)
{
    uint32_t sum = array[0] + val;
    for (size_t i = 1; i < FILTER_VALUES_MAX; i++)
    {
        sum += array[i];
        array[i - 1] = array[i];
    }
    array[FILTER_VALUES_MAX - 1] = val;
    uint16_t ret = sum / FILTER_VALUES_CNT;
    return ret;
}