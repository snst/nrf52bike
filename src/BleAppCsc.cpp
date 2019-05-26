#include "BleAppCsc.h"
#include "IUICsc.h"
#include "common.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BleAppCsc::BleAppCsc(events::EventQueue &event_queue, BLE &ble_interface, IUICsc* ui)
    : BleAppBase(event_queue, ble_interface, "CSC", BC::eCsc)
    , ui_(ui)
    , disconnect_cnt_(0)
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
            ui_->UpdateCscConnState(IUICsc::eOnline);
            ui_->Update(csc_.data_, false);
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
        ui_->UpdateCscBat(params->data[0]);
    }
}

void BleAppCsc::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    BleAppBase::OnConnected(params);
    ui_->UpdateCscConnState(IUICsc::eConnected);

}

void BleAppCsc::OnDisconnected(const Gap::DisconnectionCallbackParams_t *params)
{
    disconnect_cnt_++;
    BleAppBase::OnDisconnected(params);
    ui_->UpdateCscConnState(IUICsc::eDisconnected);
}

bool BleAppCsc::Connect()
{
    BleAppBase::Connect();
    ui_->UpdateCscConnState(IUICsc::eConnecting);
}

uint32_t BleAppCsc::GetDisconnects()
{
    return disconnect_cnt_;
}