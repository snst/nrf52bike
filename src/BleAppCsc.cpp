#include "BleAppCsc.h"
#include "ISinkCsc.h"
#include "common.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BleAppCsc::BleAppCsc(events::EventQueue &event_queue, BLE &ble_interface, ISinkCsc* sink)
    : BleAppBase(event_queue, ble_interface, "CSC"),
     csc_(sink)
{
    FindCharacteristic(0x2A5B);
}

BleAppCsc::~BleAppCsc()
{
}

void BleAppCsc::OnHVX(const GattHVXCallbackParams *params)
{
    if (params->type == BLE_HVX_NOTIFICATION)
    {
        if(csc_.ProcessData(GetMillis(), params->data, params->len)) {
            event_queue_.call(mbed::callback(&csc_, &AppCsc::UpdateGUI));
        }
    }
}
