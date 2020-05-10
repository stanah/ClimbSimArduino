#ifndef TRAINER_PERIPHERAL_H_
#define TRAINER_PERIPHERAL_H_
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <main.h>
class TrainerPeripheral {
 private:
  class MyServerCallbacks;
  class ControlCallbacks;
  static DataCallback controlCallback;

 public:
  static boolean advertising;
  static boolean connected;
  static BLECharacteristic* pDataCharacteristic;
  static BLECharacteristic* pControlCharacteristic;
  void init();
  void startAdvertising();
  void setControlCallback(DataCallback _controlCallback);
  void sendData(uint8_t* data, size_t length);
};
#endif  // TRAINER_PERIPHERAL_H_