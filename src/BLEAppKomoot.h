#ifndef BLE_APP_KOMOOT_H_
#define BLE_APP_KOMOOT_H_

#include "BLEAppBase.h"
#include "IKomootGUI.h"

class BLEAppKomoot : public BLEAppBase
{
  public:
    BLEAppKomoot(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface);
    virtual ~BLEAppKomoot();
    virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
    void UpdateGUI();
    void ProcessData(const uint8_t *data, uint32_t len);
    void OnHVX(const GattHVXCallbackParams *params);
  protected:
    uint8_t direction_;
    uint32_t distance_;
    uint8_t street_[MAX_KOMOOT_STREET_LEN];
};

#endif
