#include "BleAppKomoot.h"

BleAppKomoot::BleAppKomoot(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, ISinkKomoot *sink)
    : BleAppBase(event_queue, timer, ble_interface, "Komoot"), sink_(sink)
{
    FindCharacteristic(0xd605);
    memset(&data_, 0xFF, sizeof(data_));
    data_.street[0] = 0;
}

BleAppKomoot::~BleAppKomoot()
{
}

void BleAppKomoot::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
    BleAppBase::OnCharacteristicDescriptorsFinished(params);
    RequestNotify(true);
}

void BleAppKomoot::UpdateGUI()
{
    sink_->Update(data_);
}

bool BleAppKomoot::ProcessData(const uint8_t *data, uint32_t len)
{
    bool ret = false;
    if (len >= 9)
    {
        uint8_t direction = data[4];
        data_.direction_updated = data_.direction != direction;
        data_.direction = direction;

        uint32_t distance_m = data[5];
        distance_m |= data[6] << 8;
        distance_m |= data[7] << 16;
        distance_m |= data[8] << 24;
        data_.distance_m_updated = data_.distance_m != distance_m;
        data_.distance_m = distance_m;

        data_.street_updated = memcmp(data_.street, &data[9], len - 9);
        if (data_.street_updated)
        {
            memcpy(data_.street, &data[9], len - 9);
            data_.street[len - 9] = '\0';
        }

        INFO("komoot: dir=%u, dist=%u, len=%u, street=%s\r\n", data_.direction, data_.distance_m, len - 9, data_.street);
        ret = true;
    }
    return ret;
}

void BleAppKomoot::OnHVX(const GattHVXCallbackParams *params)
{
    if (params->type == BLE_HVX_NOTIFICATION)
    {
        if (ProcessData(params->data, params->len))
        {
            event_queue_.call(mbed::callback(this, &BleAppKomoot::UpdateGUI));
        }
    }
}