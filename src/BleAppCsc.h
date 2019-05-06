#ifndef BLE_APP_CSC_H_
#define BLE_APP_CSC_H_

#include "BleAppBase.h"
#include "AppCsc.h"

class ISinkCsc;

class BleAppCsc : public BleAppBase
{
  public:
    BleAppCsc(events::EventQueue &event_queue, BLE &ble_interface, ISinkCsc* sink);
    virtual ~BleAppCsc();
    void OnHVX(const GattHVXCallbackParams *params);
  protected:
    AppCsc csc_;
};

#endif
