#ifndef BLE_APP_KOMOOT_H_
#define BLE_APP_KOMOOT_H_

#include "BleAppBase.h"
#include "ISinkKomoot.h"

class BleAppKomoot : public BleAppBase
{
  public:
    BleAppKomoot(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, ISinkKomoot* sink);
    virtual ~BleAppKomoot();
    virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
    void UpdateGUI();
    bool ProcessData(const uint8_t *data, uint32_t len);
    void OnHVX(const GattHVXCallbackParams *params);
  protected:
    ISinkKomoot* sink_;
    ISinkKomoot::KomootData_t data_;
};

#endif
