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
        uint16_t crankEvent;
    } cscResponse_t;

    cscResponse_t last_csc_;
    uint32_t last_timestamp_ms_;
    uint16_t speed_kmhX10_;
    uint16_t cadence_;
    uint32_t total_wheel_rounds_;
    uint32_t total_travel_time_ms_;
    bool is_riding_;
    double wheel_size_cm_;
    bool is_init_;
};

#endif
