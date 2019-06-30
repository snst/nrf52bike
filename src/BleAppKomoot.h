#ifndef BLE_APP_KOMOOT_H_
#define BLE_APP_KOMOOT_H_

#include "BleAppBase.h"
#include "IUIKomoot.h"

class BleAppKomoot : public BleAppBase
{
  public:
    BleAppKomoot(events::EventQueue &event_queue, BLE &ble_interface, IUIKomoot& ui);
    virtual ~BleAppKomoot();
    bool ProcessData(const uint8_t *data, uint32_t len);
    void OnHVX(const GattHVXCallbackParams *params);
  protected:
    IUIKomoot& ui_;
    IUIKomoot::KomootData_t data_;
};

#endif
