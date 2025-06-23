#define OV5642_MINI_5MP

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
//#include "memorysaver.h"

// Set CS pin
const int CS = 10;

// Create an instance of ArduCAM
ArduCAM myCAM(OV5642, CS);

void setup() {
  delay(3000);

  uint8_t vid, pid;
  uint8_t temp;

  // Initialize Serial port
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");

  // Initialize SPI
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  SPI.begin(13, 12, 11, 10);
  Serial.println("Initialize SPI");

  // Reset the CPLD
  Wire.begin();
  Serial.println("Reset the CPLD");

  // Initialize the ArduCAM
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  Serial.println("Initialize the ArduCAM");

  if ((vid != 0x56) || (pid != 0x42)) {
    Serial.println("Can't find OV5642 module!");
    Serial.print("Camera ID - VID: 0x");
    Serial.print(vid, HEX);
    Serial.print(", PID: 0x");
    Serial.println(pid, HEX);
  } else {
    Serial.println("OV5642 detected.");
  }

  // Set camera resolution and format
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);  // You can change resolution here
  Serial.println("Set camera resolution and format");

  // Set capture mode
  myCAM.clear_fifo_flag();
  Serial.println("Set capture mode");
}

void loop() {
  uint8_t temp;
  uint32_t length;

  // Clear the buffer
  myCAM.flush_fifo();

  // Start capture
  myCAM.start_capture();
  Serial.println("start_capture.");

  // Wait for capture to finish
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    length = myCAM.read_fifo_length();
    Serial.print("Length(tmp): ");
    Serial.println(length);
    delay(1000);
  }
  Serial.println("captured.");

  // Get image length
  length = myCAM.read_fifo_length();

  Serial.print("Length: ");
  Serial.println(length);

  // Read and send image data
  myCAM.CS_LOW();
  //SPI.transfer(READ_FIFO_BURST);
  SPI.transfer(0x3D);  // Replace READ_FIFO_BURST with 0x3D

  for (uint32_t i = 0; i < length; i++) {
    temp = SPI.transfer(0x00);
    Serial.write(temp);  // Send byte via serial
  }

  myCAM.CS_HIGH();
  delay(10000);  // Delay before next capture
}
