#include "BleAppCsc.h"
#include "ISinkCsc.h"
#include "common.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BleAppCsc::BleAppCsc(events::EventQueue &event_queue, BLE &ble_interface, ISinkCsc* sink)
    : BleAppBase(event_queue, ble_interface, "CSC")
    , sink_(sink)
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
            sink_->UpdateConnState(ISinkCsc::eOnline);
            sink_->Update(csc_.data_, false);
        }
    }
}

void BleAppCsc::OnDataRead(const GattReadCallbackParams *params)
{
    if (1 == params->len)
    {
        INFO("BAT: %d\r\n", params->data[0]);
        char str[15];
        sprintf(str, "\n\nBattery: %d\n", params->data[0]);
        UILog(str);
        sink_->UpdateBat(params->data[0]);
    }
}

void BleAppCsc::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    BleAppBase::OnConnected(params);
    sink_->UpdateConnState(ISinkCsc::eConnected);

}

void BleAppCsc::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    BleAppBase::OnDisconnected(params);
    sink_->UpdateConnState(ISinkCsc::eDisconnected);
}

bool BleAppCsc::Connect()
{
    BleAppBase::Connect();
    sink_->UpdateConnState(ISinkCsc::eConnecting);
}