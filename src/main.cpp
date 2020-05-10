#include "main.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <FreeRTOS/queue.h>
#include <heltec.h>

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
#define ADC_PORT A6
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
        nextSlope = ((buf[10] * 256 + buf[9]) / 10 - 2000) *
                    2;  // Zwift is passed half the value of the displayed gradient
      }
      central.sendControl(buf, CONTROL_LENGTH);
    }
  }
}

void setup() {
  BLEDevice::init(LOCAL_NAME);
  Heltec.begin(true,   // DisplayEnable
               false,  // LoRa
               true    // Serial
  );
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_16);

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0, 0, "Initializing...");
  Heltec.display->display();

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
  Wire1.begin(SDA, SCL);

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

  Heltec.display->setFont(ArialMT_Plain_24);
}

void loop() {
  Heltec.display->clear();

  if (peripheral.connected) {
    Heltec.display->drawString(0, 0, "app: o");
  } else {
    Heltec.display->drawString(0, 0, "app: x");
    if (!peripheral.advertising) {
      peripheral.startAdvertising();
    }
  }

  if (central.doConnect == true) {
    central.connectToServer();
  }

  if (central.connected) {
    Heltec.display->drawString(80, 0, "tr: o");
  } else {
    Heltec.display->drawString(80, 0, "tr: x");
    if (central.doScan) {
      central.startScan();
    }
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
  int diff = -(nowSlope - nextSlope);

  Heltec.display->drawString(0, 32, "slope: " + String(nowSlope / 10) + "%");
  Heltec.display->display();

  if (diff > APPROVAL_DELTA) {
    digitalWrite(MOTOR_CTRL_PORT_A, HIGH);
    digitalWrite(MOTOR_CTRL_PORT_B, LOW);
  } else if (diff < -APPROVAL_DELTA) {
    digitalWrite(MOTOR_CTRL_PORT_A, LOW);
    digitalWrite(MOTOR_CTRL_PORT_B, HIGH);
  } else {
    digitalWrite(MOTOR_CTRL_PORT_A, LOW);
    digitalWrite(MOTOR_CTRL_PORT_B, LOW);
  }
  delay(250);
}
