#include "BLEAppCSC.h"
#include "BikeGUI.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BLEAppCSC::BLEAppCSC(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface) 
: BLEAppBase(event_queue, timer, ble_interface, "CSC")
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
}

void BLEAppCSC::ProcessData(const uint8_t *data, uint32_t len)
{
    // 03 6a 0a 00 00 bc 9c ad 00 1b fa**** wc=2666, we=40124, cc=173, ce=64027
    // 03 77 0b 00 00 a5 b9 ad 00 1b fa**** wc=2935, we=47525, cc=173, ce=64027
    if (len == 11)
    {
        cscResponse_t r = {0};

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

        uint32_t now = timer_.read_ms();
        uint32_t delta_ms = now - last_timestamp_;

        float crank_counter_diff = r.crankCounter - cscResponse_.crankCounter;
        float wheel_counter_diff = r.wheelCounter - cscResponse_.wheelCounter;

        FLOW("val: wc=%u, we=%u, cc=%u, ce=%u\r\n", r.wheelCounter, r.lastWheelEvent, r.crankCounter, r.lastCrankEvent);
        FLOW("dif: wc=%u, we=%u, cc=%u, ce=%u\r\n",
                (uint32_t)wheel_counter_diff,
                r.lastWheelEvent - cscResponse_.lastWheelEvent,
                (uint32_t)crank_counter_diff,
                r.lastCrankEvent - cscResponse_.lastCrankEvent);

        float wheel = 2.170f;

        cadence_ = (uint16_t)(crank_counter_diff * 60000.0f / delta_ms);
        speed_ = (uint16_t)(wheel * wheel_counter_diff * 36000.0f / delta_ms);

        INFO("now: %ums, diff=%ums, %f kmh, cadence=%u/min\r\n\r\n", now, delta_ms, speed_ / 10.0f, (uint16_t)cadence_);

        last_timestamp_ = now;
        cscResponse_ = r;

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
