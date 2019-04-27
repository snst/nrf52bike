#include "BLEAppKomoot.h"
#include "BikeGUI.h"

BLEAppKomoot::BLEAppKomoot(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface) 
: BLEAppBase(event_queue, timer, ble_interface, "Komoot")
{
    FindCharacteristic(0xd605);
}

BLEAppKomoot::~BLEAppKomoot()
{
}

void BLEAppKomoot::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    BLEAppBase::OnCharacteristicDescriptorsFinished(params);
    RequestNotify(true);
}

void BLEAppKomoot::UpdateGUI()
{
    gui_->UpdateKomootDirection(direction_);
    gui_->UpdateKomootDistance(distance_);
    gui_->UpdateKomootStreet(street_);
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
        INFO("komoot: dir=%u, dist=%u, len=%u, street=%s\r\n", direction_, distance_, len-9, street_);
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
