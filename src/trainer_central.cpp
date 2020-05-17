#include "trainer_central.h"

boolean TrainerCentral::doConnect = false;
boolean TrainerCentral::doScan = false;
boolean TrainerCentral::connected = false;
BLEAdvertisedDevice *TrainerCentral::myDevice = nullptr;
BLERemoteCharacteristic *TrainerCentral::pControlRemoteCharacteristic = nullptr;
DataCallback TrainerCentral::dataCallback = nullptr;
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class TrainerCentral::MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(TRAINER_SERVICE_UUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

class TrainerCentral::TrainerConnectionCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) { Serial.println("[Central] Trainer connected"); }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    doScan = true;
    BLEDevice::unsetClientDevice(pclient->getPeerAddress());
    Serial.println("[Central] Trainer disconnected");
  }
};
void TrainerCentral::trainerDataCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                                         uint8_t *pData, size_t length, bool isNotify) {
  if (dataCallback != nullptr) {
    dataCallback(pData, length);
  } else {
    Serial.println("DataCharacteristic is null");
  }
}

bool TrainerCentral::connectToServer() {
  Serial.print("[Central] Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  BLEDevice::setClientDevice(myDevice->getAddress());
  pClient->setClientCallbacks(new TrainerConnectionCallback());
  pClient->connect(myDevice);
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(TRAINER_SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("[Central] Failed to find our service UUID: ");
    Serial.println(TRAINER_SERVICE_UUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pDataRemoteCharacteristic = pRemoteService->getCharacteristic(TRAINER_DATA_CHAR_UUID);
  if (pDataRemoteCharacteristic == nullptr) {
    Serial.print("[Central] Failed to find our characteristic UUID: ");
    Serial.println(TRAINER_DATA_CHAR_UUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  pControlRemoteCharacteristic = pRemoteService->getCharacteristic(TRAINER_CONTROL_CHAR_UUID);
  if (pControlRemoteCharacteristic == nullptr) {
    Serial.print("[Central] Failed to find our characteristic UUID: ");
    Serial.println(TRAINER_CONTROL_CHAR_UUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  if (pDataRemoteCharacteristic->canNotify())
    pDataRemoteCharacteristic->registerForNotify(trainerDataCallback);
  connected = true;
  doConnect = false;
  Serial.println("[Central] Services/Characteristics discovered");
  return true;
}

void TrainerCentral::init() {
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  doScan = true;
}  // End of setup.

void TrainerCentral::startScan() {
  BLEDevice::getScan()->start(5);
  return;
}

void TrainerCentral::sendControl(uint8_t *data, size_t length) {
  if (connected == true && pControlRemoteCharacteristic != nullptr) {
    pControlRemoteCharacteristic->writeValue(data, length);
  }
}

void TrainerCentral::setDataCallback(DataCallback _dataCallback) { dataCallback = _dataCallback; }