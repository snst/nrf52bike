#include "BleAppCsc.h"
#include "ISinkCsc.h"

#define FLAG_WHEEL_PRESENT (1)
#define FLAG_CRANK_PRESENT (2)

BleAppCsc::BleAppCsc(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, ISinkCsc* csc_data)
    : BleAppBase(event_queue, timer, ble_interface, "CSC"),
     csc_(csc_data)
{
    FindCharacteristic(0x2A5B);
}

BleAppCsc::~BleAppCsc()
{
}

void BleAppCsc::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    BleAppBase::OnCharacteristicDescriptorsFinished(params);
    RequestNotify(true);
/*
            Gap::ConnectionParams_t new_params;
        new_params.minConnectionInterval = 1000;
        new_params.maxConnectionInterval = 1200;
        new_params.slaveLatency = 0;
        new_params.connectionSupervisionTimeout = 500;
        ble_error_t ret = ble_.gap().updateConnectionParams(GetConnectionHandle(), &new_params);
        INFO("UPDATE11: 0x%x\r\n", ret);*/
}

void BleAppCsc::OnHVX(const GattHVXCallbackParams *params)
{
    if (params->type == BLE_HVX_NOTIFICATION)
    {
        if(csc_.ProcessData(timer_.read_ms(), params->data, params->len)) {
            event_queue_.call(mbed::callback(&csc_, &AppCsc::UpdateGUI));
        }
    }
}
