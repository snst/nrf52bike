#include "BLEAppBase.h"

#define DISCONNECTED_HANDLE (0xFFFFu)

BLEAppBase::BLEAppBase(events::EventQueue &event_queue, Timer &timer, BLE &ble_interface, char *name) : event_queue_(event_queue),
                                                                                                        timer_(timer),
                                                                                                        ble_(ble_interface),
                                                                                                        _post_init_cb(),
                                                                                                        desc2902_(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0)),
                                                                                                        found_characteristic(false),
                                                                                                        found_desc2902_(false),
                                                                                                        found_device_(false),
                                                                                                        app_callback_(NULL),
                                                                                                        connection_handle_(DISCONNECTED_HANDLE)
{
  strcpy(name_, name);
}

BLEAppBase::~BLEAppBase()
{
}

void BLEAppBase::SetGUI(BikeGUI *gui)
{
  gui_ = gui;
}

bool BLEAppBase::HaveFoundDevice()
{
  return found_device_;
}

const Gap::Handle_t &BLEAppBase::GetConnectionHandle()
{
  return connection_handle_;
}

bool BLEAppBase::Connect()
{
  if (found_device_)
  {
    ble_error_t err = ble_.gap().connect(device_addr_, device_addr_type_, NULL, NULL);
    INFO("~BLEAppBase::Connect() => %s, err=0x%x\r\n", name_, err);
    return true;
  }
  return false;
}

void BLEAppBase::OnConnected(const Gap::ConnectionCallbackParams_t *params)
{
  connection_handle_ = params->handle;
  INFO("~BLEAppBase::OnConnected() => %s, handle=0x%x\r\n", name_, connection_handle_);
  if (!FoundCharacteristic())
  {
    StartServiceDiscovery();
  }
  else
  {
    OnServiceDiscoveryFinished(connection_handle_);
  }
}

void BLEAppBase::OnDisconnected(const Gap::DisconnectionCallbackParams_t *param)
{
  INFO("~BLEAppBase::OnDisconnected() => %s, handle=0x%x, reason=0x%x\r\n", name_, param->handle, param->reason);
  connection_handle_ = DISCONNECTED_HANDLE;
  //event_queue_.call_in(500, mbed::callback(this, &BLEAppBase::Connect));
}

bool BLEAppBase::IsConnected()
{
  return DISCONNECTED_HANDLE != connection_handle_;
}

void BLEAppBase::OnServiceFound(const DiscoveredService *service)
{
  FLOW("~BLEAppBase::OnServiceFound() => %s UUID-%x\r\n", name_, service->getUUID().getShortUUID());
  //DBG("~BLEAppBase::OnServiceFound() LONG-%x %x %x %x %x\r\n", service->getUUID().getBaseUUID()[0], service->getUUID().getBaseUUID()[1], service->getUUID().getBaseUUID()[2], service->getUUID().getBaseUUID()[3], service->getUUID().getBaseUUID()[4]);
}

void BLEAppBase::OnServiceCharacteristicFound(const DiscoveredCharacteristic *param)
{
  INFO("~BLEAppBase::OnServiceCharacteristicFound() => %s, UUID-%x valueHandle=%u, declHandle=%u, props[%x]\r\n", name_, param->getUUID().getShortUUID(), param->getValueHandle(), param->getDeclHandle(), (uint8_t)param->getProperties().broadcast());
  //DBG("~BLEAppBase::OnServiceCharacteristicFound() LONG-%x %x %x %x %x\r\n", param->getUUID().getBaseUUID()[0], param->getUUID().getBaseUUID()[1], param->getUUID().getBaseUUID()[2], param->getUUID().getBaseUUID()[3], param->getUUID().getBaseUUID()[4]);
  if (param->getUUID().getShortUUID() == characteristic_id)
  {
    INFO("~BLEAppBase:: => %s -> characteristic 0x%x\r\n", name_, characteristic_id);
    characteristic_ = *param;
    found_characteristic = true;
  }
}

void BLEAppBase::OnServiceDiscoveryFinished(Gap::Handle_t handle)
{
  INFO("~BLEAppBase::OnServiceDiscoveryFinished() => %s, handle=0x%x\r\n", name_, handle);

  if (!FoundDescNotify())
  {
    INFO("~~BLEAppBase::OnServiceDiscoveryFinished() => %s, !FoundDescNotify\r\n", name_);
    if (FoundCharacteristic())
    {
      StartCharacteristicDescriptorsDiscovery(GetCharacteristic());
    }
  }
  else
  {
    OnCharacteristicDescriptorsFinished(NULL);
  }
}

void BLEAppBase::StartServiceDiscovery()
{
  ble_error_t error = ble_.gattClient().launchServiceDiscovery(
      connection_handle_,
      as_cb(&Self::OnServiceFound),
      as_cb(&Self::OnServiceCharacteristicFound));

  INFO("~BLEAppBase::StartServiceDiscovery() => %s, err=0x%x\r\n", name_, error);
}

void BLEAppBase::OnCharacteristicDescriptorsFound(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params)
{
  DBG("~BLEAppBase::OnCharacteristicDescriptorsFound() => %s, UUID %x\r\n", name_, params->descriptor.getUUID().getShortUUID());
  if (params->descriptor.getUUID().getShortUUID() == 0x2902)
  {
    INFO("BLEAppBase::OnCharacteristicDescriptorsFound() => %s  ->  found 0x2902\r\n", name_);
    desc2902_ = params->descriptor;
    found_desc2902_ = true;
    ble_.gattClient().terminateCharacteristicDescriptorDiscovery(params->characteristic);
  }
}

void BLEAppBase::OnAppReady()
{
  INFO("~BLEAppBase::OnAppReady() => %s\r\n", name_);
  if (NULL != app_callback_)
  {
    app_callback_->OnAppReady(this);
  }
}

void BLEAppBase::OnCharacteristicDescriptorsFinished(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params)
{
  INFO("~BLEAppBase::OnCharacteristicDescriptorsFinished() => %s\r\n", name_);
  OnAppReady();
}

void BLEAppBase::StartCharacteristicDescriptorsDiscovery(DiscoveredCharacteristic &characteristic)
{
  ble_error_t err = ble_.gattClient().discoverCharacteristicDescriptors(characteristic, as_cb(&Self::OnCharacteristicDescriptorsFound), as_cb(&Self::OnCharacteristicDescriptorsFinished));
  INFO("~BLEAppBase::StartCharacteristicDescriptorsDiscovery() => %s, err=0x%x\r\n", name_, err);
}

void BLEAppBase::RequestNotify(bool enable)
{
  uint16_t value = enable ? BLE_HVX_NOTIFICATION : 0;
  ble_error_t err = ble_.gattClient().write(
      GattClient::GATT_OP_WRITE_CMD,
      desc2902_.getConnectionHandle(),
      desc2902_.getAttributeHandle(),
      sizeof(uint16_t),
      (uint8_t *)&value);
  INFO("~BLEAppBase::RequestNotify(%d) => %s, attrHandle=%u, ret=0x%x\r\n", value, name_, desc2902_.getAttributeHandle(), err);
}

void BLEAppBase::ReadNotifyStatus(DiscoveredCharacteristicDescriptor &desc) {}

void BLEAppBase::OnDataRead(const GattReadCallbackParams *params) {}

void BLEAppBase::OnHVX(const GattHVXCallbackParams *params)
{
}

void BLEAppBase::on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb)
{
  _post_init_cb = cb;
}

void BLEAppBase::SetDeviceAddress(const Gap::AdvertisementCallbackParams_t *params)
{
  memcpy(&device_addr_, &params->peerAddr, sizeof(device_addr_));
  device_addr_type_ = params->addressType;
  found_device_ = true;
}

bool BLEAppBase::HasAddress(const BLEProtocol::AddressBytes_t &peerAddr)
{
  return 0 == memcmp(&device_addr_, &peerAddr, sizeof(device_addr_));
}

void BLEAppBase::FindCharacteristic(uint16_t id)
{
  characteristic_id = id;
}

bool BLEAppBase::FoundCharacteristic()
{
  return found_characteristic;
}

bool BLEAppBase::FoundDescNotify()
{
  return found_desc2902_;
}

DiscoveredCharacteristic &BLEAppBase::GetCharacteristic()
{
  return characteristic_;
}

void BLEAppBase::SetAppCallback(IAppCallback *cb)
{
  app_callback_ = cb;
}
