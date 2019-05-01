#ifndef BLE_APP_CSC_H_
#define BLE_APP_CSC_H_

#include "BleAppBase.h"
#include "AppCsc.h"

class IDataCsc;

class BleAppCsc : public BleAppBase
{
  public:
    BleAppCsc(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, IDataCsc* csc_data_);
    virtual ~BleAppCsc();
    virtual void OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params);
    void OnHVX(const GattHVXCallbackParams *params);
  protected:
    AppCsc csc_;
};

#endif
