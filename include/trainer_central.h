#ifndef TRAINER_CENTRAL_H_
#define TRAINER_CENTRAL_H_

#include <Arduino.h>
#include <BLEDevice_tweek.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <main.h>
class TrainerCentral {
 private:
  BLERemoteCharacteristic *pDataRemoteCharacteristic;

  class MyAdvertisedDeviceCallbacks;
  class TrainerConnectionCallback;
  static DataCallback dataCallback;
  static void trainerDataCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData,
                                  size_t length, bool isNotify);
  static BLERemoteCharacteristic *pControlRemoteCharacteristic;

 public:
  static boolean doConnect;
  static boolean doScan;
  static boolean connected;
  static BLEAdvertisedDevice *myDevice;

  bool connectToServer();
  void init();
  void startScan();
  void sendControl(uint8_t *data, size_t length);
  void setDataCallback(DataCallback dataCallback);
};
#endif