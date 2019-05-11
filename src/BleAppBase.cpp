#include "BleAppBase.h"
#include "tracer.h"

#define DISCONNECTED_HANDLE (0xFFFFu)

BleAppBase::BleAppBase(events::EventQueue &event_queue, BLE &ble_interface, char *name) : event_queue_(event_queue),
                                                                                                        ble_(ble_interface),
                                                                                                        _post_init_cb(),
                                                                                                        desc2902_(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0)),
                                                                                                        found_characteristic_bat_(false),
                                                                                                        found_characteristic_(false),
                                                                                                        found_desc2902_(false),
                                                                                                        found_device_(false),
                                                                                                        app_callback_(NULL),
                                                                                                        connection_handle_(DISCONNECTED_HANDLE)
                                                                                                        
{
  strcpy(name_, name);
}

BleAppBase::~BleAppBase()
{
}

bool BleAppBase::HaveFoundDevice()
{
  return found_device_;
}

const Gap::Handle_t &BleAppBase::GetConnectionHandle()
{
  return connection_handle_;
}

bool BleAppBase::Connect()
{
  if (found_device_)
  {
    ble_error_t err = ble_.gap().connect(device_addr_, device_addr_type_, NULL, NULL);
    INFO("~BleAppBase::Connect() => %s, err=0x%x\r\n", name_, err);
    UILog("Connect ");
    UILog(name_);
    return true;
  }
  return false;
}

void BleAppBase::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
  connection_handle_ = params->handle;
  INFO("~BleAppBase::OnConnected() => %s, handle=0x%x\r\n", name_, connection_handle_);
  UILog(".ok.");
  if (!FoundCharacteristic())
  {
    StartServiceDiscovery();
  }
  else
  {
    OnServiceDiscoveryFinished(connection_handle_);
  }
}

void BleAppBase::OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
{
  INFO("~BleAppBase::OnDisconnected() => %s, handle=0x%x, reason=0x%x\r\n", name_, param->handle, param->reason);
  connection_handle_ = DISCONNECTED_HANDLE;
  //event_queue_.call_in(500, mbed::callback(this, &BleAppBase::Connect));
}

bool BleAppBase::IsConnected()
{
  return DISCONNECTED_HANDLE != connection_handle_;
}

void BleAppBase::OnServiceFound(const DiscoveredService *service)
{
  FLOW("~BleAppBase::OnServiceFound() => %s UUID-%x\r\n", name_, service->getUUID().getShortUUID());
  //DBG("~BleAppBase::OnServiceFound() LONG-%x %x %x %x %x\r\n", service->getUUID().getBaseUUID()[0], service->getUUID().getBaseUUID()[1], service->getUUID().getBaseUUID()[2], service->getUUID().getBaseUUID()[3], service->getUUID().getBaseUUID()[4]);
}

void BleAppBase::OnServiceCharacteristicFound(const DiscoveredCharacteristic *param)
{
  FLOW("~BleAppBase::OnServiceCharacteristicFound() => %s, UUID-%x valueHandle=%u, declHandle=%u, props[%x]\r\n", name_, param->getUUID().getShortUUID(), param->getValueHandle(), param->getDeclHandle(), (uint8_t)param->getProperties().broadcast());
  //DBG("~BleAppBase::OnServiceCharacteristicFound() LONG-%x %x %x %x %x\r\n", param->getUUID().getBaseUUID()[0], param->getUUID().getBaseUUID()[1], param->getUUID().getBaseUUID()[2], param->getUUID().getBaseUUID()[3], param->getUUID().getBaseUUID()[4]);
  if (param->getUUID().getShortUUID() == characteristic_id)
  {
    INFO("~BleAppBase:: => %s -> characteristic 0x%x\r\n", name_, characteristic_id);
    characteristic_ = *param;
    found_characteristic_ = true;
    UILog("char.");
  }
  if (param->getUUID().getShortUUID() == 0x2a19u)
  {
    INFO("~BleAppBase:: => %s -> characteristic bat\r\n", name_);
    characteristic_bat_ = *param;
    found_characteristic_bat_ = true;
    UILog("char bat.");
  }
}

void BleAppBase::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
  INFO("~BleAppBase::OnServiceDiscoveryFinished() => %s, handle=0x%x\r\n", name_, handle);

  if (!FoundDescNotify())
  {
    INFO("~~BleAppBase::OnServiceDiscoveryFinished() => %s, !FoundDescNotify\r\n", name_);
    if (FoundCharacteristic())
    {
      StartCharacteristicDescriptorsDiscovery(GetCharacteristic());
    }
  }
  else
  {
    OnAppReady();
  }
}

void BleAppBase::StartServiceDiscovery()
{
  ble_error_t error = ble_.gattClient().launchServiceDiscovery(
      connection_handle_,
      as_cb(&Self::OnServiceFound),
      as_cb(&Self::OnServiceCharacteristicFound));

  INFO("~BleAppBase::StartServiceDiscovery() => %s, err=0x%x\r\n", name_, error);
}

void BleAppBase::OnCharacteristicDescriptorsFound(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
  DBG("~BleAppBase::OnCharacteristicDescriptorsFound() => %s, UUID %x\r\n", name_, params->descriptor.getUUID().getShortUUID());
  if (params->descriptor.getUUID().getShortUUID() == 0x2902)
  {
    INFO("BleAppBase::OnCharacteristicDescriptorsFound() => %s  ->  found 0x2902\r\n", name_);
    desc2902_ = params->descriptor;
    found_desc2902_ = true;
    ble_.gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
  }
}

void BleAppBase::OnAppReady()
{
  INFO("~BleAppBase::OnAppReady() => %s\r\n", name_);

  RequestNotify();

  event_queue_.call_in(100, mbed::callback(this, &BleAppBase::RequestBatteryLevel));

  if (NULL != app_callback_)
  {
    app_callback_->OnAppReady(this);
  }
}

bool BleAppBase::RequestBatteryLevel()
{
  if (found_characteristic_bat_) {
    ble_error_t err = characteristic_bat_.read(0);
    INFO("~BleAppBase::RequestBatteryLevel(), ret=0x%x\r\n", err);
  }
  return found_characteristic_bat_;
}


void BleAppBase::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
  INFO("~BleAppBase::OnCharacteristicDescriptorsFinished() => %s\r\n", name_);
  UILog("desc.");
  OnAppReady();
}

void BleAppBase::StartCharacteristicDescriptorsDiscovery(DiscoveredCharacteristic &characteristic)
{
  ble_error_t err = ble_.gattClient().discoverCharacteristicDescriptors(characteristic, as_cb(&Self::OnCharacteristicDescriptorsFound), as_cb(&Self::OnCharacteristicDescriptorsFinished));
  INFO("~BleAppBase::StartCharacteristicDescriptorsDiscovery() => %s, err=0x%x\r\n", name_, err);
}

void BleAppBase::RequestNotify()
{
  bool enable = true;
  uint16_t value = enable ? BLE_HVX_NOTIFICATION : 0;
  ble_error_t err = ble_.gattClient().write(
      GattClient::GATT_OP_WRITE_CMD,
      desc2902_.getConnectionHandle(),
      desc2902_.getAttributeHandle(),
      sizeof(uint16_t),
      (uint8_t *)&value);
  INFO("~BleAppBase::RequestNotify(%d) => %s, attrHandle=%u, ret=0x%x\r\n", value, name_, desc2902_.getAttributeHandle(), err);
}

void BleAppBase::ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc) {}

void BleAppBase::OnDataRead(const GattReadCallbackParams *params) {}

void BleAppBase::OnHVX(const GattHVXCallbackParams *params)
{
}

void BleAppBase::on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb)
{
  _post_init_cb = cb;
}

void BleAppBase::SetDeviceAddress(const Gap::AdvertisementCallbackParams_t *params)
{
  memcpy(&device_addr_, &params->peerAddr, sizeof(device_addr_));
  device_addr_type_ = params->addressType;
  found_device_ = true;
}

bool BleAppBase::HasAddress(const BLEProtocol::AddressBytes_t &peerAddr)
{
  return 0 == memcmp(&device_addr_, &peerAddr, sizeof(device_addr_));
}

void BleAppBase::FindCharacteristic(uint16_t id)
{
  characteristic_id = id;
}

bool BleAppBase::FoundCharacteristic()
{
  return found_characteristic_;
}

bool BleAppBase::FoundDescNotify()
{
  return found_desc2902_;
}

DiscoveredCharacteristic &BleAppBase::GetCharacteristic()
{
  return characteristic_;
}

void BleAppBase::SetAppCallback(IAppCallback *cb)
{
  app_callback_ = cb;
}
