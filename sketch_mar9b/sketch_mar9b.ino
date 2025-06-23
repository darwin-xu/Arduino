#include <Wire.h>
#include <SPI.h>
#include <ArduCAM.h>
#include <memorysaver.h>

// Use this definition ONLY for the OV5642 model
#define OV5642_MINI_5MP_PLUS

// Pin definitions for Arduino Nano ESP32
#define CS_PIN     5   // A4
#define RESET_PIN  6   // A3
#define SCK_PIN    41  // D6
#define MISO_PIN   40  // D7
#define MOSI_PIN   39  // D8

// I2C pins
#define I2C_SDA    3   // D2
#define I2C_SCL    2   // D3

ArduCAM myCAM(OV5642, CS_PIN);

void setup() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  Serial.begin(115200);
  delay(1000);
  Serial.println("ArduCAM OV5642 Test - Nano ESP32");

  Wire.begin(I2C_SDA, I2C_SCL);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  // Reset camera
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
  delay(100);

  // Initialize camera
  myCAM.write_reg(ARDUCHIP_MODE, 0x00);  // Switch to MCU mode
  uint8_t vid, pid;
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV5642_CHIPID_LOW, &pid);

  if ((vid != 0x56) || (pid != 0x42)) {
    Serial.println("Can't find OV5642 camera!");
    while (1);
  } else {
    Serial.println("OV5642 detected.");
  }

  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
  delay(1000);
  Serial.println("Camera initialized.");
}

void loop() {
  Serial.println("Capturing image...");
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    delay(10);
  }

  Serial.println("Capture done. Sending image...");

  uint32_t length = myCAM.read_fifo_length();
  if (length >= 0x07FFFF) {
    Serial.println("Image too large!");
    return;
  }

  if (length == 0) {
    Serial.println("Image length is zero.");
    return;
  }

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  while (length--) {
    uint8_t val = SPI.transfer(0x00);
    Serial.write(val);  // Send raw JPEG to serial (view in ArduCAM host app)
  }

  myCAM.CS_HIGH();
  Serial.println("\nImage sent.\n");

  delay(5000);  // Wait before next capture
}
