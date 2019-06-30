#include "BleAppKomoot.h"
#include "common.h"

BleAppKomoot::BleAppKomoot(events::EventQueue &event_queue, BLE &ble_interface, IUIKomoot &ui)
    : BleAppBase(event_queue, ble_interface, "Komoot", BC::eKomoot), ui_(ui)
{
    FindCharacteristic(0xd605);
    memset(&data_, 0xFF, sizeof(data_));
    data_.street[0] = 0u;
}

BleAppKomoot::~BleAppKomoot()
{
}

bool BleAppKomoot::ProcessData(const uint8_t *data, uint32_t len)
{
    bool ret = false;
    if (len >= 9u)
    {
        uint8_t direction = data[4];
        data_.direction = direction;

        uint32_t distance_m = data[5];
        distance_m |= data[6] << 8u;
        distance_m |= data[7] << 16u;
        distance_m |= data[8] << 24u;
        data_.distance_m = RoundDistance(distance_m);

        memcpy(data_.street, &data[9], len - 9u);
        data_.street[len - 9] = '\0';

#if 0
        static uint16_t dist = 650;
        dist-=10;
        config_.distance_m = dist;
        if(dist > 5000) {
            dist = 650;
            config_.direction = 0xFF;
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
            ui_.Update(data_, false);
        }
    }
}
