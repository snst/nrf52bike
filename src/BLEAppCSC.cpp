#include "BLEAppCSC.h"
#include "BikeGUI.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BLEAppCSC::BLEAppCSC(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface)
    : BLEAppBase(event_queue, timer, ble_interface, "CSC"), is_init_(false), total_wheel_rounds_(0)
    , total_travel_time_ms_(0), wheel_size_cm_(217.0), is_riding_(false)
{
    FindCharacteristic(0x2A5B);
}

BLEAppCSC::~BLEAppCSC()
{
}

void BLEAppCSC::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    BLEAppBase::OnCharacteristicDescriptorsFinished(params);
    RequestNotify(true);
}

void BLEAppCSC::UpdateGUI()
{
    double dist_cm = wheel_size_cm_ * total_wheel_rounds_;
    gui_->UpdateIsRiding(is_riding_);
    gui_->UpdateSpeed(speed_kmhX10_);
    gui_->UpdateCadence(cadence_);
    gui_->UpdateTravelDistance((uint32_t)dist_cm); // cm
    gui_->UpdateTravelTime(total_travel_time_ms_ / 1000); // sec
}

void BLEAppCSC::ProcessData(const uint8_t *data, uint32_t len)
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

        uint32_t now = timer_.read_ms();
        if (is_init_)
        {
            uint32_t delta_ms = now - last_timestamp_ms_;
            uint32_t wheel_counter_diff = csc.wheelCounter - last_csc_.wheelCounter;
            uint32_t wheel_event_diff = csc.wheelEvent - last_csc_.wheelEvent;
            double delta_sec = wheel_event_diff / 1024.0;
            double speed_kmh = 0.0f;

            is_riding_ = wheel_counter_diff > 0;
            total_wheel_rounds_ += wheel_counter_diff;

            if (delta_sec > 0.0f)
            {
                // cm/sec * 3600 / 100 / 1000
                speed_kmhX10_ = (uint16_t)((wheel_size_cm_ * wheel_counter_diff * 3.6f) / delta_sec / 10.0f);
            } else {
                speed_kmhX10_ = 0;
            }

            uint32_t crank_counter_diff = csc.crankCounter - last_csc_.crankCounter;
            uint32_t crank_event_diff = csc.crankEvent - last_csc_.crankEvent;
            double crank_delta_sec = crank_event_diff / 1024.0;

            if (crank_delta_sec > 0.0f)
            {
                cadence_ = crank_counter_diff * 60.0f / crank_delta_sec;
            }

            INFO("wc=%u, we=%u, dwc=%u, dwe=%u, ms=%u, s=%f, r=%u\r\n",
                csc.wheelCounter, csc.wheelEvent, wheel_counter_diff, wheel_event_diff,
                delta_ms, delta_sec, total_wheel_rounds_);

            if (is_riding_) {
                total_travel_time_ms_ += delta_ms;
            } 

            INFO("now: %ums, diff=%ums, %f kmh, cadence=%u/min, time=%u\r\n\r\n"
            , now, delta_ms, speed_kmhX10_ / 10.0f, (uint16_t)cadence_, total_travel_time_ms_/1000);
        }

        last_timestamp_ms_ = now;
        last_csc_ = csc;
        is_init_ = true;

        event_queue_.call(mbed::callback(this, &BLEAppCSC::UpdateGUI));
    }
}

void BLEAppCSC::OnHVX(const GattHVXCallbackParams *params)
{
    if (params->type == BLE_HVX_NOTIFICATION)
    {
        ProcessData(params->data, params->len);
    }
}
