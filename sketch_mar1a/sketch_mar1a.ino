#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

void setup() {
  Serial.begin(9600);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  // Reset the scale to 0
  scale.tare();
  
  Serial.println("Place known weight on scale");
  delay(5000);
  
  // Calibration factor will be the (reading) / (known weight)
  // Adjust this calibration factor as needed
  scale.set_scale(2280.f);    // this value is obtained by calibrating the scale with known weights
}

void loop() {
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1);
  Serial.println(" kg"); 
  delay(1000);
}