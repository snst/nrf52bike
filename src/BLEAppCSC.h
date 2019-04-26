#ifndef BLE_APP_CSC_H_
#define BLE_APP_CSC_H_

#include "BLEAppBase.h"

class BLEAppCSC : public BLEAppBase
{
  public:
    BLEAppCSC(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface);
    virtual ~BLEAppCSC();
    virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
    void UpdateGUI();
    void ProcessData(const uint8_t *data, uint32_t len);
    void OnHVX(const GattHVXCallbackParams *params);

  protected:
    typedef struct cscResponse
    {
        uint8_t flags;
        uint32_t wheelCounter;
        uint16_t wheelEvent;
        uint16_t crankCounter;
        uint16_t lastCrankEvent;
    } cscResponse_t;

    cscResponse_t last_csc;
    uint32_t last_timestamp_;
    uint16_t speed_;
    uint16_t cadence_;
    uint32_t total_wheel_rounds_;
    bool is_init_;
};

#endif
