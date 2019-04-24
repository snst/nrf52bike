#ifndef BLE_MANAGER_H_
#define BLE_MANAGER_H_

#include "BLEManagerBase.h"
#include "BLEAppCSC.h"
#include "BLEAppKomoot.h"

class BLEManager : public BLEManagerBase
{
public:
  BLEAppCSC csc_app;
  BLEAppKomoot komoot_app;

  BLEManager(BLE &ble_interface, BikeGUI *gui);
  virtual ~BLEManager();
  virtual void OnConnected(const Gap::ConnectionCallbackParams_t *params);
  virtual void OnDisconnected(const Gap::DisconnectionCallbackParams_t *param);
  virtual void OnDataRead(const GattReadCallbackParams *params);
  virtual void OnHVX(const GattHVXCallbackParams *params);
  virtual void OnServiceDiscoveryFinished(Gap::Handle_t handle);
  //virtual void OnAdvertisement(const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnFoundService16(uint16_t id, const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnFoundService128(const uint8_t *id, const Gap::AdvertisementCallbackParams_t *params);
  virtual void OnScanStopped();
  virtual void OnAppReady(BLEAppBase *app);
  BLEAppBase* GetAppWithConnectionHandle(Gap::Handle_t handle);
  void CheckScanStop();
  void ConnectDevices();
};

#endif