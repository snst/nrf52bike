#include "BikeComputer.h"
#include "tracer.h"

/**
 * Handle discovery of the GATT server.
 *
 * First the GATT server is discovered in its entirety then each readable
 * characteristic is read and the client register to characteristic
 * notifications or indication when available. The client report server
 * indications and notification until the connection end.
 */
GattClientProcess::GattClientProcess(BLE &be) : _connection_handle(),
                                                _characteristics(NULL),
                                                _it(NULL),
                                                _descriptor_handle(0),
                                                _ble_interface(be),
                                                _event_queue(NULL)
{
}

GattClientProcess::~GattClientProcess()
{
    stop();
}

void GattClientProcess::init(events::EventQueue *queue)
{
    INFO("+GattClientProcess::init()\r\n");
    _event_queue = queue;

    _ble_interface.gap().setEventHandler(this);
    _ble_interface.gap().onConnection(makeFunctionPointer(this, &Self::when_connected));
    _ble_interface.gap().onDisconnection(makeFunctionPointer(this, &Self::when_disconnected));

    _ble_interface.onEventsToProcess(
        makeFunctionPointer(this, &GattClientProcess::schedule_ble_events));

    ble_error_t error = _ble_interface.init(
        this, &GattClientProcess::when_init_complete);

    //_ble_interface.initializeSecurity(false, false);

    INFO("-GattClientProcess::init(), res=0x%x\r\n", error);
}

void GattClientProcess::when_connected(const Gap::ConnectionCallbackParams_t *connection_event)
{
    INFO("when_connected()\r\n");
    _connection_handle = connection_event->handle;
    _event_queue->call(mbed::callback(this, &GattClientProcess::start));
}

void GattClientProcess::when_disconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    INFO("Disconnected.\r\n");
}
/**
     * Schedule processing of events from the BLE middleware in the event queue.
     */
void GattClientProcess::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event)
{
    _event_queue->call(mbed::callback(&event->ble, &BLE::processEvents));
}

void GattClientProcess::when_advertisment_received(const Gap::AdvertisementCallbackParams_t *params)
{
    INFO("~when_advertisment_received peerAddr[%02x %02x %02x %02x %02x %02x] rssi %d, isScanResponse %u, AdvertisementType %u, peerAddrType %u.\r\n",
         params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0],
         params->rssi, params->isScanResponse, params->type, params->peerAddrType);

    if (params->peerAddr[5] == 0xf4)
    {
        INFO("~when_advertisment_received() -> eFoundDevice\r\n");
        _ble_interface.gap().stopScan();
        _ble_interface.gap().connect(params->peerAddr, params->addressType, NULL, NULL);
        //            memcpy(&device_addr, &params->peerAddr, sizeof(device_addr));
        //            state = eFoundDevice;
    }
}

void GattClientProcess::when_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    INFO("+when_init_complete() err=0x%x\r\n", event->error);

    //gap.setScanParams(500, 400, 0, false);
    ble_error_t err = _ble_interface.gap().startScan(this, &GattClientProcess::when_advertisment_received);

    INFO("-when_init_complete() err=0x%x\r\n", err);
}

/**
     * Start the discovery process.
     *
     * @param[in] client The GattClient instance which will discover the distant
     * GATT server.
     * @param[in] connection_handle Reference of the connection to the GATT
     * server which will be discovered.
     */
void GattClientProcess::start()
{
    INFO("GattClientProcess::start()\r\n");
    // setup the event handlers called during the process
    _ble_interface.gattClient().onDataWritten().add(as_cb(&Self::when_descriptor_written));
    _ble_interface.gattClient().onHVX().add(as_cb(&Self::when_characteristic_changed));

    // The discovery process will invoke when_service_discovered when a
    // service is discovered, when_characteristic_discovered when a
    // characteristic is discovered and when_service_discovery_ends once the
    // discovery process has ended.
    _ble_interface.gattClient().onServiceDiscoveryTermination(as_cb(&Self::when_service_discovery_ends));
    ble_error_t error = _ble_interface.gattClient().launchServiceDiscovery(
        _connection_handle,
        as_cb(&Self::when_service_discovered),
        as_cb(&Self::when_characteristic_discovered));

    if (error)
    {
        INFO("Error %u returned by _ble_interface.gattClient().launchServiceDiscovery.\r\n", error);
        return;
    }

    INFO("Client process started: initiate service discovery.\r\n");
}

/**
     * Stop the discovery process and clean the instance.
     */
void GattClientProcess::stop()
{
    // unregister event handlers
    _ble_interface.gattClient().onDataWritten().detach(as_cb(&Self::when_descriptor_written));
    _ble_interface.gattClient().onHVX().detach(as_cb(&Self::when_characteristic_changed));
    _ble_interface.gattClient().onServiceDiscoveryTermination(NULL);

    // remove discovered characteristics
    clear_characteristics();

    // clean up the instance
    _connection_handle = 0;
    _characteristics = NULL;
    _it = NULL;
    _descriptor_handle = 0;

    INFO("Client process stopped.\r\n");
}


////////////////////////////////////////////////////////////////////////////////
// Service and characteristic discovery process.

/**
     * Handle services discovered.
     *
     * The GattClient invokes this function when a service has been discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
void GattClientProcess::when_service_discovered(const DiscoveredService *discovered_service)
{
    // print information of the service discovered
    INFO("Service discovered: value = ");
    print_uuid(discovered_service->getUUID());
    INFO(", start = %u, end = %u.\r\n",
         discovered_service->getStartHandle(),
         discovered_service->getEndHandle());
}

/**
     * Handle characteristics discovered.
     *
     * The GattClient invoke this function when a characteristic has been
     * discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
void GattClientProcess::when_characteristic_discovered(const DiscoveredCharacteristic *discovered_characteristic)
{
    // print characteristics properties
    INFO("\tCharacteristic discovered: uuid = ");
    print_uuid(discovered_characteristic->getUUID());
    INFO(", properties = ");
    print_properties(discovered_characteristic->getProperties());
    INFO(
        ", decl handle = %u, value handle = %u, last handle = %u.\r\n",
        discovered_characteristic->getDeclHandle(),
        discovered_characteristic->getValueHandle(),
        discovered_characteristic->getLastHandle());
    if (discovered_characteristic->getUUID().getShortUUID() == 0x2A5B)
        {
            INFO("  -> eFoundServiceCharacteristic_0x2A5B\r\n");
            characteristic_0x2A5B = *discovered_characteristic;
            //_ble_interface.gattClient().terminateServiceDiscovery();
            //state = eFoundServiceCharacteristic_0x2A5B;
        }
}

/**
     * Handle termination of the service and characteristic discovery process.
     *
     * The GattClient invokes this function when the service and characteristic
     * discovery process ends.
     *
     * @see GattClient::onServiceDiscoveryTermination
     */
void GattClientProcess::when_service_discovery_ends(Gap::Handle_t connection_handle)
{
    INFO("when_service_discovery_ends()\r\n");
    discover_descriptors(characteristic_0x2A5B);
    /*
    if (!_characteristics)
    {
        INFO("No characteristics discovered, end of the process.\r\n");
        return;
    }

    INFO("All services and characteristics discovered, process them.\r\n");

    // reset iterator and start processing characteristics in order
    _it = NULL;
    _event_queue->call(mbed::callback(this, &Self::process_next_characteristic));
    */
}

////////////////////////////////////////////////////////////////////////////////
// Processing of characteristics based on their properties.

/**
     * Process the characteristics discovered.
     *
     * - If the characteristic is readable then read its value and print it. Then
     * - If the characteristic can emit notification or indication then discover
     * the characteristic CCCD and subscribe to the server initiated event.
     * - Otherwise skip the characteristic processing.
     */
void GattClientProcess::process_next_characteristic(void)
{
    if (!_it)
    {
        _it = _characteristics;
    }
    else
    {
        _it = _it->next;
    }

    while (_it)
    {
        Properties_t properties = _it->value.getProperties();

        if (properties.read())
        {
            read_characteristic(_it->value);
            return;
        }
        else if (properties.notify() || properties.indicate())
        {
            discover_descriptors(_it->value);
            return;
        }
        else
        {
            INFO(
                "Skip processing of characteristic %u\r\n",
                _it->value.getValueHandle());
            _it = _it->next;
        }
    }

    INFO("All characteristics discovered have been processed.\r\n");
}

/**
     * Initate the read of the characteristic in input.
     *
     * The completion of the operation will happens in when_characteristic_read()
     */
void GattClientProcess::read_characteristic(const DiscoveredCharacteristic &characteristic)
{
    INFO("Initiating read at %u.\r\n", characteristic.getValueHandle());
    ble_error_t error = characteristic.read(
        0, as_cb(&Self::when_characteristic_read));

    if (error)
    {
        INFO(
            "Error: cannot initiate read at %u due to %u\r\n",
            characteristic.getValueHandle(), error);
        stop();
    }
}

/**
     * Handle the reception of a read response.
     *
     * If the characteristic can emit notification or indication then start the
     * discovery of the the characteristic descriptors then subscribe to the
     * server initiated event by writing the CCCD discovered. Otherwise start
     * the processing of the next characteristic discovered in the server.
     */
void GattClientProcess::when_characteristic_read(const GattReadCallbackParams *read_event)
{
    INFO("when_characteristic_read(): Characteristic value at %u equal to: ", read_event->handle);
    for (size_t i = 0; i < read_event->len; ++i)
    {
        INFO("0x%02X ", read_event->data[i]);
    }
    INFO(".\r\n");

    Properties_t properties = _it->value.getProperties();

    if (properties.notify() || properties.indicate())
    {
        discover_descriptors(_it->value);
    }
    else
    {
        //process_next_characteristic();
    }
}

/**
     * Initiate the discovery of the descriptors of the characteristic in input.
     *
     * When a descriptor is discovered, the function when_descriptor_discovered
     * is invoked.
     */
void GattClientProcess::discover_descriptors(const DiscoveredCharacteristic &characteristic)
{
    INFO("Initiating descriptor discovery of %u.\r\n", characteristic.getValueHandle());

    _descriptor_handle = 0;
    ble_error_t error = characteristic.discoverDescriptors(
        as_cb(&Self::when_descriptor_discovered),
        as_cb(&Self::when_descriptor_discovery_ends));

    if (error)
    {
        INFO(
            "Error: cannot initiate discovery of %04X due to %u.\r\n",
            characteristic.getValueHandle(), error);
        stop();
    }
}

/**
     * Handle the discovery of the characteristic descriptors.
     *
     * If the descriptor found is a CCCD then stop the discovery. Once the
     * process has ended subscribe to server initiated events by writing the
     * value of the CCCD.
     */
void GattClientProcess::when_descriptor_discovered(const DiscoveryCallbackParams_t *event)
{
    INFO("\tDescriptor discovered at %u, UUID: ", event->descriptor.getAttributeHandle());
    print_uuid(event->descriptor.getUUID());
    INFO(".\r\n");

    if (event->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
            INFO("BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG\r\n");
        _descriptor_handle = event->descriptor.getAttributeHandle();
        _ble_interface.gattClient().terminateCharacteristicDescriptorDiscovery(
            event->characteristic);
    }
}

/**
     * If a CCCD has been found subscribe to server initiated events by writing
     * its value.
     */
void GattClientProcess::when_descriptor_discovery_ends(const TerminationCallbackParams_t *event)
{
    INFO("when_descriptor_discovery_ends()\r\n");
    // shall never happen but happen with android devices ...
    // process the next charateristic
    if (!_descriptor_handle)
    {
        INFO("\tWarning: characteristic with notify or indicate attribute without CCCD.\r\n");
        //process_next_characteristic();
        return;
    }

    Properties_t properties = _it->value.getProperties();

    uint16_t cccd_value =
        (properties.notify() << 0) | (properties.indicate() << 1);

    ble_error_t error = _ble_interface.gattClient().write(
        GattClient::GATT_OP_WRITE_REQ,
        _connection_handle,
        _descriptor_handle,
        sizeof(cccd_value),
        reinterpret_cast<uint8_t *>(&cccd_value));

        INFO(
            "write of CCCD %u , val=0x%x, due to %u.\r\n",
            _descriptor_handle, cccd_value, error);

}

/**
     * Called when the CCCD has been written.
     */
void GattClientProcess::when_descriptor_written(const GattWriteCallbackParams *event)
{
    // should never happen
    if (!_descriptor_handle)
    {
        INFO("\tError: received write response to unsolicited request.\r\n");
        stop();
        return;
    }

    INFO("\tCCCD at %u written.\r\n", _descriptor_handle);
    _descriptor_handle = 0;
    //process_next_characteristic();
}

/**
     * Print the updated value of the characteristic.
     *
     * This function is called when the server emits a notification or an
     * indication of a characteristic value the client has subscribed to.
     *
     * @see GattClient::onHVX()
     */
void GattClientProcess::when_characteristic_changed(const GattHVXCallbackParams *event)
{
    INFO("Change on attribute %u: new value = ", event->handle);
    for (size_t i = 0; i < event->len; ++i)
    {
        INFO("0x%02X ", event->data[i]);
    }
    INFO(".\r\n");
}

/**
     * Add a discovered characteristic into the list of discovered characteristics.
     */
bool GattClientProcess::add_characteristic(const DiscoveredCharacteristic *characteristic)
{
    /*
    DiscoveredCharacteristicNode *new_node =
        new (std::nothrow) DiscoveredCharacteristicNode(*characteristic);

    if (new_node == false)
    {
        INFO("Error while allocating a new characteristic.\r\n");
        return false;
    }

    if (_characteristics == NULL)
    {
        _characteristics = new_node;
    }
    else
    {
        DiscoveredCharacteristicNode *c = _characteristics;
        while (c->next)
        {
            c = c->next;
        }
        c->next = new_node;
    }
*/
    return true;
}

/**
     * Clear the list of discovered characteristics.
     */
void GattClientProcess::clear_characteristics(void)
{
    DiscoveredCharacteristicNode *c = _characteristics;

    while (c)
    {
        DiscoveredCharacteristicNode *n = c->next;
        delete c;
        c = n;
    }
}

/**
     * Print the value of a UUID.
     */
void GattClientProcess::print_uuid(const UUID &uuid)
{
    const uint8_t *uuid_value = uuid.getBaseUUID();

    // UUIDs are in little endian, print them in big endian
    for (size_t i = 0; i < uuid.getLen(); ++i)
    {
        INFO("%02X", uuid_value[(uuid.getLen() - 1) - i]);
    }
}

/**
     * Print the value of a characteristic properties.
     */
void GattClientProcess::print_properties(const Properties_t &properties)
{
    const struct
    {
        bool (Properties_t::*fn)() const;
        const char *str;
    } prop_to_str[] = {
        {&Properties_t::broadcast, "broadcast"},
        {&Properties_t::read, "read"},
        {&Properties_t::writeWoResp, "writeWoResp"},
        {&Properties_t::write, "write"},
        {&Properties_t::notify, "notify"},
        {&Properties_t::indicate, "indicate"},
        {&Properties_t::authSignedWrite, "authSignedWrite"}};

    INFO("[");
    for (size_t i = 0; i < (sizeof(prop_to_str) / sizeof(prop_to_str[0])); ++i)
    {
        if ((properties.*(prop_to_str[i].fn))())
        {
            INFO(" %s", prop_to_str[i].str);
        }
    }
    INFO(" ]");
}
