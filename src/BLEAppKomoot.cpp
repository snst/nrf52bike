#include "BLEAppKomoot.h"
#include "IKomootGui.h"

BLEAppKomoot::BLEAppKomoot(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface) : BLEAppBase(event_queue, timer, ble_interface)
{
    FindCharacteristic(0x2A5B);
}

BLEAppKomoot::~BLEAppKomoot()
{
}

void BLEAppKomoot::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
    BLEAppBase::OnConnected(params);
    INFO("~BLEAppKomoot::OnConnected() => CSC\r\n");
}

void BLEAppKomoot::OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
{
    BLEAppBase::OnDisconnected(param);
    INFO("~BLEAppKomoot::OnDisconnected() handle=0x%x, reason=0x%x\r\n", param->handle, param->reason);
    event_queue_.call(mbed::callback(this, &BLEAppBase::Connect));
}

void BLEAppKomoot::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
    BLEAppBase::OnServiceDiscoveryFinished(handle);
}

void BLEAppKomoot::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    BLEAppBase::OnCharacteristicDescriptorsFinished(params);
    RequestNotify(true);
}

void BLEAppKomoot::UpdateGUI()
{
    gui_->UpdateDirection(direction_);
    gui_->UpdateDistance(distance_);
    gui_->UpdateStreet(street_);
}

void BLEAppKomoot::ProcessData(const uint8_t *data, uint32_t len)
{
    if(len >= 9) 
    {
        direction_ = data[4];
        distance_ = data[5];
        distance_ |= data[6] <<8 ;
        distance_ |= data[7]<<16;
        distance_ |= data[8]<<24;
        memset(street_, 0, sizeof(street_));
        memcpy(street_, &data[9], len-9);
        INFO("komoot: dir=%u, dist=%u, len=%u, street=%s\r\n", direction_, distance_, len-9, street);
        event_queue_.call(mbed::callback(this, &BLEAppKomoot::UpdateGUI));
    }
}

void BLEAppKomoot::OnHVX(const GattHVXCallbackParams *params)
{
    if (params->type == BLE_HVX_NOTIFICATION)
    {
        ProcessData(params->data, params->len);
    }
}
