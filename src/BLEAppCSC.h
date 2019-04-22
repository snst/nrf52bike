#ifndef BLE_APP_CSC_H_
#define BLE_APP_CSC_H_

#include "BLEAppBase.h"
#include "val.h"
#include "BikeGui.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

class BLEAppCSC : public BLEAppBase
{
  protected:
    typedef struct bikeResponse
    {
        uint8_t flags;
        uint32_t wheelCounter;
        uint16_t lastWheelEvent;
        uint16_t crankCounter;
        uint16_t lastCrankEvent;
    } bikeResponse_t;

    bikeResponse_t bikeResponse_;
    uint32_t last_timestamp_ = 0;
    val_uint8_t speed_;

  public:
    BLEAppCSC(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface) : BLEAppBase(event_queue, timer, ble_interface)
    {
        FindCharacteristic(0x2A5B);
    }

    virtual ~BLEAppCSC()
    {
    }

    virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params)
    {
        BLEAppBase::OnConnected(params);
        INFO("~BLEAppCSC::OnConnected() => CSC\r\n");
    }

    virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
    {
        BLEAppBase::OnDisconnected(param);
        INFO("~BLEAppCSC::OnDisconnected() handle=0x%x, reason=0x%x\r\n", param->handle, param->reason);
        event_queue_.call(mbed::callback(this, &BLEAppBase::Connect));
    }

    virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle)
    {
        BLEAppBase::OnServiceDiscoveryFinished(handle);
    }

    virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
    {
        BLEAppBase::OnCharacteristicDescriptorsFinished(params);
        RequestNotify(true);
    }

    void UpdateSpeed()
    {
        if (new_val(speed_))
        {
            gui_->UpdateSpeed(speed_.current);
        }
    }

    void ProcessCscData(const uint8_t *data, uint32_t len)
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

            uint32_t now = timer_.read_ms();
            uint32_t delta_ms = now - last_timestamp_;

            uint32_t crank_counter_diff = r.crankCounter - bikeResponse_.crankCounter;
            uint32_t wheel_counter_diff = r.wheelCounter - bikeResponse_.wheelCounter;

            FLOW("val: wc=%u, we=%u, cc=%u, ce=%u\r\n", r.wheelCounter, r.lastWheelEvent, r.crankCounter, r.lastCrankEvent);
            FLOW("dif: wc=%u, we=%u, cc=%u, ce=%u\r\n",
                 wheel_counter_diff,
                 r.lastWheelEvent - bikeResponse_.lastWheelEvent,
                 crank_counter_diff,
                 r.lastCrankEvent - bikeResponse_.lastCrankEvent);

            float wheel = 2.170f;

            float crank_per_min = ((float)crank_counter_diff) * 60000.0f / delta_ms;

            float wheel_per_min = (((float)wheel_counter_diff) * 60000.0f / delta_ms);
            float kmh = wheel * wheel_per_min * 60.0f / 1000.0f;
            speed_.current = (uint8_t)kmh;

            INFO("now: %ums, diff=%ums, %f kmh, cadence=%f/min\r\n\r\n", now, delta_ms, kmh, wheel_per_min, crank_per_min);

            last_timestamp_ = now;
            bikeResponse_ = r;

            event_queue_.call(mbed::callback(this, &BLEAppCSC::UpdateSpeed));
        }
    }

    void OnHVX(const GattHVXCallbackParams *params)
    {

        if (params->type == BLE_HVX_NOTIFICATION)
        {
            ProcessCscData(params->data, params->len);
        }
        /*
        if (params->type == BLE_HVX_NOTIFICATION)
    {
        if (params->handle == characteristic_csc.getValueHandle())
        {
            process_csc_data(params->data, params->len, t, speed);
        }
        else if (params->handle == characteristic_komoot.getValueHandle())
        {
            process_komoot_data(params->data, params->len, komoot_dir, komoot_dist, (uint8_t *)street, sizeof(street));
        }
    }
*/
    }
};

#endif
