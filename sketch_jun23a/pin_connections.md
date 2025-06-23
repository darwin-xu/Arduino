# Updated Pin Summary

## Arduino Nano ESP32 Pin Connections:

| **ArduCam Pin** | **Nano ESP32 Pin** | **Notes**                   |
| --------------- | ------------------ | --------------------------- |
| VCC             | 3.3V               | 3.3 V power                 |
| GND             | GND                | Common ground               |
| CS              | D10                | Chip-select                 |
| MOSI            | D11                | SPI MOSI                    |
| MISO            | D12                | SPI MISO                    |
| SCK             | D13                | SPI SCK                     |
| SDA             | A4                 | SCCB (I²C) data (Wire SDA)  |
| SCL             | A5                 | SCCB (I²C) clock (Wire SCL) |


The updated code now uses the correct pin definitions for the Arduino Nano ESP32. The SPI communication will use the hardware SPI pins (D11, D12, D13) and I2C will use the default pins (A4, A5). This should work properly with your Arduino Nano ESP32 board.

## Make sure to:

- Install the ArduCAM library from the Library Manager
- Connect all pins according to the corrected wiring diagram
- If using WiFi features, update the WiFi credentials in the code
- Use the Serial Monitor to send commands like "capture" or "stream"

## About the ArduCAM library:
- The library ZIP is downloaded from: https://github.com/ArduCAM/Arduino
- To install it in the arduino IDE, first extract it. Then recompress the ArduCAM in the folder using "zip -r ArduCAM.zip ArduCAM" and the install it. 

## Ref:
- https://docs.arduino.cc/software/ide/
- https://docs.arduino.cc/hardware/nano-esp32/
- https://docs.arduino.cc/tutorials/nano-esp32/cheat-sheet/
- https://docs.arduino.cc/resources/pinouts/ABX00083-full-pinout.pdf
- https://docs.arduino.cc/resources/datasheets/ABX00083-datasheet.pdf
- https://www.arducam.com/arducam-5mp-plus-spi-cam-arduino-ov5642.html