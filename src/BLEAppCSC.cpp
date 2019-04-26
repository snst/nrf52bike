#include "BLEAppCSC.h"
#include "BikeGUI.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BLEAppCSC::BLEAppCSC(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface)
    : BLEAppBase(event_queue, timer, ble_interface, "CSC"), is_init_(false), total_wheel_rounds_(0)
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
    gui_->UpdateSpeed(speed_);
    gui_->UpdateCadence(cadence_);
    gui_->UpdateTravelDistance(total_wheel_rounds_);
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
            csc.lastCrankEvent = data[p++];
            csc.lastCrankEvent |= data[p++] << 8;
        }

        uint32_t now = timer_.read_ms();
        if (is_init_)
        {
            double wheel_size = 2.170;
            uint32_t delta_ms = now - last_timestamp_;
            uint32_t wheel_counter_diff = csc.wheelCounter - last_csc.wheelCounter;
            uint32_t wheel_event_diff = csc.wheelEvent - last_csc.wheelEvent;
            double delta_sec = wheel_event_diff / 1024.0;
            double speed = 0.0f;

            total_wheel_rounds_ += wheel_counter_diff;

            if (delta_sec > 0.0f)
            {
                speed = (wheel_size * wheel_counter_diff * 3.6f) / delta_sec;
            }

            INFO("wc=%u, we=%u, dwc=%u, dwe=%u, ms=%u, s=%f, r=%u, kmh=%f\r\n",
            csc.wheelCounter, csc.wheelEvent, wheel_counter_diff, wheel_event_diff,
            delta_ms, delta_sec, total_wheel_rounds_, speed
            );

            speed_ = (uint16_t)(speed*10);

/*
            float crank_counter_diff = csc.crankCounter - last_csc.crankCounter;

            INFO("val: wc=%u, we=%u, cc=%u, ce=%u\r\n", csc.wheelCounter, csc.wheelEvent, csc.crankCounter, csc.lastCrankEvent);
            INFO("dif: wc=%u, we=%u, cc=%u, ce=%u\r\n",
                 (uint32_t)wheel_counter_diff,
                 csc.wheelEvent - last_csc.wheelEvent,
                 (uint32_t)crank_counter_diff,
                 csc.lastCrankEvent - last_csc.lastCrankEvent);

            cadence_ = (uint16_t)(crank_counter_diff * 60000.0f / delta_ms);
            */
            //speed_ = (uint16_t)(wheel * wheel_counter_diff * 36000.0f / delta_ms);

            INFO("now: %ums, diff=%ums, %f kmh, cadence=%u/min\r\n\r\n", now, delta_ms, speed_ / 10.0f, (uint16_t)cadence_);
        }

        last_timestamp_ = now;
        last_csc = csc;
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
