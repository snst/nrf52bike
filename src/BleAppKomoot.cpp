#include "BleAppKomoot.h"

BleAppKomoot::BleAppKomoot(events::EventQueue &event_queue, BLE &ble_interface, ISinkKomoot *sink)
    : BleAppBase(event_queue, ble_interface, "Komoot"), sink_(sink)
{
    FindCharacteristic(0xd605);
    memset(&data_, 0xFF, sizeof(data_));
    data_.street[0] = 0;
}

BleAppKomoot::~BleAppKomoot()
{
}

bool BleAppKomoot::ProcessData(const uint8_t *data, uint32_t len)
{
    bool ret = false;
    if (len >= 9)
    {
        uint8_t direction = data[4];
        data_.direction = direction;

        uint32_t distance_m = data[5];
        distance_m |= data[6] << 8;
        distance_m |= data[7] << 16;
        distance_m |= data[8] << 24;
        data_.distance_m = distance_m;

        memcpy(data_.street, &data[9], len - 9);
        data_.street[len - 9] = '\0';

#if 0
        static uint16_t dist = 650;
        dist-=10;
        data_.distance_m = dist;
        if(dist > 5000) {
            dist = 650;
            data_.direction = 0xFF;
        }
#endif
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
            sink_->Update(data_, false);
        }
    }
}
