#include "main.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <FreeRTOS/queue.h>

#include "trainer_central.h"
#include "trainer_peripheral.h"

#define APPROVAL_DELTA (5)
#define DISTANCE_OFFSET (100)
// #define MOTOR_MAX (300)
// #define MOTOR_MAX (0)
#define ADC_PORT (34)

#define CONTROL_LENGTH (13)

static TrainerCentral central;
static TrainerPeripheral peripheral;

QueueHandle_t xControlQueue;
int nextSlope = 0;
int nowSlope = 0;

static void controlCallback(uint8_t* data, size_t length) {
  if (length != CONTROL_LENGTH) {
    Serial.println("invalid control length");
  } else {
    xQueueSendToBack(xControlQueue, data, 0);
  }
}

static void dataCallback(uint8_t* data, size_t length) { peripheral.sendData(data, length); }

void vControlTask(void* pvParameters) {
  BaseType_t xStatus;
  uint8_t buf[CONTROL_LENGTH];
  while (1) {
    xStatus = xQueueReceive(xControlQueue, &buf, portMAX_DELAY);
    if (xStatus == pdPASS) {
      if (buf[4] == 0x33) {
        nextSlope = (buf[10] * 256 + buf[9]) / 10 - 2000;
        Serial.print("next slope:");
        Serial.println(nextSlope);
      }
      central.sendControl(buf, CONTROL_LENGTH);
    }
  }
}

void setup() {
  BLEDevice::init(LOCAL_NAME);
  Serial.begin(115200);
  xControlQueue = xQueueCreate(10, CONTROL_LENGTH);
  if (xControlQueue != NULL) {
    xTaskCreate(vControlTask, "Task", 4096, NULL, 1, (TaskHandle_t*)NULL);
  }
  central.init();
  peripheral.init();

  central.setDataCallback((DataCallback)dataCallback);
  peripheral.setControlCallback((DataCallback)controlCallback);
}

void loop() {
  if (central.doConnect == true) {
    central.connectToServer();
    if (central.connected) {
      Serial.println("[Central] connected to the BLE Server.");
      Serial.println("[Peripheral] Start Advertising.");
      peripheral.startAdvertising();
    } else {
      Serial.println("[Central] failed to connect to the server.");
    }
  }

  if (central.connected) {
  } else if (central.doScan) {
    central.startScan();
  }

  nowSlope = analogRead(ADC_PORT);
  Serial.print("now slope:");
  Serial.println(nowSlope);
  int diff = -(nowSlope - nextSlope);
  if (diff > APPROVAL_DELTA) {
    Serial.println("up");
    // actuatorDriver.up();
  } else if (diff < -APPROVAL_DELTA) {
    Serial.println("down");
    // actuatorDriver.down();
  } else {
    Serial.println("stop");
    // actuatorDriver.stop();
  }
  delay(1000);
}