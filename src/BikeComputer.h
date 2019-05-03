#ifndef BLE_MANAGER_H_
#define BLE_MANAGER_H_

#include "BleManagerBase.h"
#include "BleAppCsc.h"
#include "BleAppKomoot.h"

class BikeComputer : public BleManagerBase
{
public:
  BleAppCsc csc_app;
  BleAppKomoot komoot_app;

  BikeComputer(BLE &ble_interface, UIMain *ui);
  virtual ~BikeComputer();
  virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
  virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
  virtual void OnDataRead(const GattReadCallbackParams *params);
  virtual void OnHVX(const GattHVXCallbackParams *params);
  virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle);
  //virtual void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnScanStopped();
  virtual void OnAppReady(BleAppBase *app);
  BleAppBase* GetAppWithConnectionHandle(Gap::Handle_t handle);
  void CheckScanStop();
  void ConnectDevices();
};

#endif
