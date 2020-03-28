#ifndef TRAINER_INFO_H_
#define TRAINER_INFO_H_

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// TACX TRAINER INFO
#define LOCAL_NAME "Tacx Neo dummy"
#define APPEARANCE_GENERIC_CYCLING (0x480)
// TACX TRAINER UUID
static BLEUUID TRAINER_SERVICE_UUID("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID TRAINER_DATA_CHAR_UUID("6E40FEC2-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID TRAINER_CONTROL_CHAR_UUID("6E40FEC3-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID CP_SERVICE_UUID("1818");
static BLEUUID CSC_SERVICE_UUID("1816");
static BLEUUID DEVICE_INFO_SERVICE_UUID("180A");
typedef void (*DataCallback)(uint8_t* data, size_t length);
#endif  // TRAINER_INFO_H_