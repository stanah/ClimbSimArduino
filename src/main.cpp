#include "main.h"

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <BLEDevice_fix.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Fonts/FreeSans9pt7b.h>
#include <FreeRTOS/queue.h>
#include <Wire.h>

// #define SENSOR_TYPE_POTENTIOMETER

#ifndef SENSOR_TYPE_POTENTIOMETER
#include <Adafruit_VL53L0X.h>
#endif

#include "trainer_central.h"
#include "trainer_peripheral.h"

#define APPROVAL_DELTA (5)

#define MOTOR_CTRL_PORT_A (0)
#define MOTOR_CTRL_PORT_B (2)

#define CONTROL_LENGTH (13)
#define AVG_CNT (10)

#define UP (1)
#define DOWN (-1)
#define STOP (0)

#ifdef SENSOR_TYPE_POTENTIOMETER
#define ADC_PORT A6
#define SLOPE_ALPHA (0.2)
#define SLOPE_BETA (300)
#else
#define SLOPE_ALPHA (1)
#define SLOPE_BETA (100)  // Default distance to ToF sensor
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#endif

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RST_OLED);

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

void vCentralTask(void* pvParameters) {
  central.init();
  central.setDataCallback((DataCallback)dataCallback);
  while (1) {
    if (!central.connected) {
      if (central.doScan) {
        central.startScan();
      }
      if (central.doConnect == true) {
        central.connectToServer();
      }
    }
    vTaskDelay(2000);
  }
}
void vPeripheralTask(void* pvParameters) {
  peripheral.init();
  peripheral.setControlCallback((DataCallback)controlCallback);
  while (1) {
    if (!peripheral.connected && !peripheral.advertising) {
      Serial.println("startAdvertising");
      peripheral.startAdvertising();
    }
    vTaskDelay(2000);
  }
}

uint8_t toNextSlope() {
#ifndef SENSOR_TYPE_POTENTIOMETER
  VL53L0X_RangingMeasurementData_t measure;
#endif
  int sum = 0;
  int cnt = 0;
  while (cnt < AVG_CNT) {
#ifdef SENSOR_TYPE_POTENTIOMETER
    sum += analogRead(ADC_PORT);
    cnt++;
#else
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      sum += measure.RangeMilliMeter;
      cnt++;
    }
#endif
  }
  nowSlope = (sum / cnt) * SLOPE_ALPHA - SLOPE_BETA;

  int diff = -(nowSlope - nextSlope);
  if (diff > APPROVAL_DELTA) {
    digitalWrite(MOTOR_CTRL_PORT_A, HIGH);
    digitalWrite(MOTOR_CTRL_PORT_B, LOW);
    return UP;
  }
  if (diff < -APPROVAL_DELTA) {
    digitalWrite(MOTOR_CTRL_PORT_A, LOW);
    digitalWrite(MOTOR_CTRL_PORT_B, HIGH);
    return DOWN;
  }
  digitalWrite(MOTOR_CTRL_PORT_A, LOW);
  digitalWrite(MOTOR_CTRL_PORT_B, LOW);
  return STOP;
}

void setup() {
  BLEDevice::init(LOCAL_NAME);
  Serial.begin(115200);
  Wire.begin(SDA_OLED, SCL_OLED);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1)
      ;
  }
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0, 25);
  display.print("Initializing...");
  display.display();

  xControlQueue = xQueueCreate(10, CONTROL_LENGTH);
  if (xControlQueue != NULL) {
    xTaskCreate(vControlTask, "Task", 4096, NULL, 1, (TaskHandle_t*)NULL);
  }
  xTaskCreate(vCentralTask, "BLECentral", 4096, NULL, 1, (TaskHandle_t*)NULL);
  xTaskCreate(vPeripheralTask, "BLECentral", 4096, NULL, 1, (TaskHandle_t*)NULL);

#ifdef SENSOR_TYPE_POTENTIOMETER
  pinMode(ADC_PORT, ANALOG);
#else
  Wire1.begin(SDA, SCL);
  if (!lox.begin(0x30, false, &Wire1)) {
    display.clearDisplay();
    display.setCursor(0, 25);
    display.print("VL53L0X allocation failed");
    display.display();
    while (1)
      ;
  }
#endif

  pinMode(MOTOR_CTRL_PORT_A, OUTPUT);
  pinMode(MOTOR_CTRL_PORT_B, OUTPUT);

  // initialize motor to slope 0
  while (toNextSlope() != STOP) {
    display.clearDisplay();
    display.setCursor(0, 25);
    display.print("Initializing...");
    display.setCursor(0, 50);
    display.print("Slope: " + String(nowSlope / 10) + " %");
    display.display();
    delay(250);
  }
}

void loop() {
  display.clearDisplay();

  display.setCursor(0, 25);
  if (peripheral.connected) {
    display.print("App: o");
  } else {
    display.print("App: x");
  }

  display.setCursor(64, 25);
  if (central.connected) {
    display.print("Tr: o");
  } else {
    display.print("Tr: x");
  }
  display.setCursor(0, 50);
  display.print(String(nowSlope / 10) + " % -> " + String(nextSlope / 10) + "%");
  display.display();

  toNextSlope();

  delay(250);
}
