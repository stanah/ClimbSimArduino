#include "main.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <FreeRTOS/queue.h>

// #define SENSOR_TYPE_POTENTIOMETER

#ifndef SENSOR_TYPE_POTENTIOMETER
#include <VL53L0X.h>
#include <Wire.h>
#endif

#include "trainer_central.h"
#include "trainer_peripheral.h"

#define APPROVAL_DELTA (5)

#define MOTOR_CTRL_PORT_A (0)
#define MOTOR_CTRL_PORT_B (2)

#define CONTROL_LENGTH (13)
#define AVG_CNT (10)

#ifdef SENSOR_TYPE_POTENTIOMETER
#define SLOPE_ALPHA (0.2)
#define SLOPE_BETA (300)
#else
#define SLOPE_ALPHA (0.1)
#define SLOPE_BETA (5)
VL53L0X sensor;
#endif
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
        nextSlope = ((buf[10] * 256 + buf[9]) / 10 - 2000) * 2;
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

#ifdef SENSOR_TYPE_POTENTIOMETER
  pinMode(ADC_PORT, ANALOG);
#else
  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {
    }
  }
  sensor.startContinuous();
#endif

  pinMode(MOTOR_CTRL_PORT_A, OUTPUT);
  pinMode(MOTOR_CTRL_PORT_B, OUTPUT);

  // initialize motor to slope 0
  while (1) {
    int sum = 0;
    int cnt = 0;
    for (int i = 0; i < AVG_CNT; i++) {
#ifdef SENSOR_TYPE_POTENTIOMETER
      sum += analogRead(ADC_PORT);
      cnt++;
#else
      int milli = sensor.readRangeContinuousMillimeters();
      if (sensor.timeoutOccurred()) {
        Serial.print(" TIMEOUT");
      } else {
        sum += milli;
        cnt++;
      }
#endif
    }
    nowSlope = (sum / cnt) * SLOPE_ALPHA - SLOPE_BETA;
    Serial.print("now slope:");
    Serial.println(nowSlope);
    int diff = -(nowSlope - 0);
    if (diff > APPROVAL_DELTA) {
      digitalWrite(MOTOR_CTRL_PORT_A, HIGH);
      digitalWrite(MOTOR_CTRL_PORT_B, LOW);
    } else if (diff < -APPROVAL_DELTA) {
      digitalWrite(MOTOR_CTRL_PORT_A, LOW);
      digitalWrite(MOTOR_CTRL_PORT_B, HIGH);
    } else {
      digitalWrite(MOTOR_CTRL_PORT_A, LOW);
      digitalWrite(MOTOR_CTRL_PORT_B, LOW);
      break;
    }
    delay(250);
  }
  Serial.println("initialized");
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

  int sum = 0;
  int cnt = 0;
  for (int i = 0; i < AVG_CNT; i++) {
#ifdef SENSOR_TYPE_POTENTIOMETER
    sum += analogRead(ADC_PORT);
    cnt++;
#else
    int milli = sensor.readRangeContinuousMillimeters();
    if (sensor.timeoutOccurred()) {
      Serial.print("TIMEOUT");
    } else {
      sum += milli;
      cnt++;
    }
#endif
  }
  nowSlope = (sum / cnt) * SLOPE_ALPHA - SLOPE_BETA;

  Serial.print("now slope:");
  Serial.println(nowSlope);
  int diff = -(nowSlope - nextSlope);
  if (diff > APPROVAL_DELTA) {
    // Serial.println("up");
    digitalWrite(MOTOR_CTRL_PORT_A, HIGH);
    digitalWrite(MOTOR_CTRL_PORT_B, LOW);
  } else if (diff < -APPROVAL_DELTA) {
    // Serial.println("down");
    digitalWrite(MOTOR_CTRL_PORT_A, LOW);
    digitalWrite(MOTOR_CTRL_PORT_B, HIGH);
  } else {
    digitalWrite(MOTOR_CTRL_PORT_A, LOW);
    digitalWrite(MOTOR_CTRL_PORT_B, LOW);
    // Serial.println("stop");
  }
  delay(250);
}
