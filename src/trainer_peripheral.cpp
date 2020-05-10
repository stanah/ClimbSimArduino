#include "trainer_peripheral.h"

#include <BLE2902.h>
boolean TrainerPeripheral::connected = false;
boolean TrainerPeripheral::advertising = false;
BLECharacteristic *TrainerPeripheral::pDataCharacteristic = nullptr;
BLECharacteristic *TrainerPeripheral::pControlCharacteristic = nullptr;
DataCallback TrainerPeripheral::controlCallback = nullptr;

class TrainerPeripheral::MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    connected = true;
    advertising = false;
    Serial.println("[Peripheral] Application connected");
  };
  void onDisconnect(BLEServer *pServer) {
    connected = false;
    Serial.println("[Peripheral] Application disconnected");
  }
};

class TrainerPeripheral::ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      uint8_t *uint8t = (uint8_t *)(value.c_str());
      if (controlCallback) controlCallback(uint8t, value.length());
    } else {
      Serial.println("cant write now");
    }
  }
};

void TrainerPeripheral::init() {
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(TRAINER_SERVICE_UUID);
  pDataCharacteristic =
      pService->createCharacteristic(TRAINER_DATA_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pControlCharacteristic = pService->createCharacteristic(
      TRAINER_CONTROL_CHAR_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pDataCharacteristic->addDescriptor(new BLE2902());
  pControlCharacteristic->setCallbacks(new ControlCallbacks());
  BLEService *pCpService = pServer->createService(CP_SERVICE_UUID);
  BLEService *pCscService = pServer->createService(CSC_SERVICE_UUID);
  BLEService *pDeviceInfoService = pServer->createService(DEVICE_INFO_SERVICE_UUID);
  pDeviceInfoService->start();
  pCpService->start();
  pCscService->start();
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->setScanResponse(true);

  BLEAdvertisementData advertisementData;
  advertisementData.setName(LOCAL_NAME);
  advertisementData.setAppearance(APPEARANCE_GENERIC_CYCLING);
  advertisementData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
  pAdvertising->setAdvertisementData(advertisementData);

  BLEAdvertisementData scanResponseData;
  scanResponseData.setCompleteServices(TRAINER_SERVICE_UUID);
  scanResponseData.setCompleteServices(CP_SERVICE_UUID);
  scanResponseData.setCompleteServices(CSC_SERVICE_UUID);
  scanResponseData.setCompleteServices(DEVICE_INFO_SERVICE_UUID);
  pAdvertising->setScanResponseData(scanResponseData);

}  // End of setup.

void TrainerPeripheral::startAdvertising() {
  advertising = true;
  BLEDevice::startAdvertising();
}

void TrainerPeripheral::setControlCallback(DataCallback _controlCallback) {
  controlCallback = _controlCallback;
}
void TrainerPeripheral::sendData(uint8_t *data, size_t length) {
  if (pDataCharacteristic != nullptr) {
    pDataCharacteristic->setValue(data, length);
    pDataCharacteristic->notify(true);
  }
}
