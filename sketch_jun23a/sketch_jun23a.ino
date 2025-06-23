#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// ArduCam library - you'll need to install this from Library Manager
#include "ArduCAM.h"
#include "ov5642_regs.h"

// Pin definitions for Arduino Nano ESP32
#define CS_PIN 10    // D10
// SDA and SCL use default I2C pins (A4, A5)

// Create ArduCAM instance for OV5642
ArduCAM myCAM(OV5642, CS_PIN);

// WiFi credentials (optional - for web server)
const char* ssid = "a-tp";
const char* password = "opop9090";

// Web server on port 80
WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C (uses default pins A4=SDA, A5=SCL)
  Wire.begin();
  
  // Initialize SPI (uses default pins: D11=MOSI, D12=MISO, D13=SCK)
  SPI.begin();
  
  // Initialize CS pin
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  // Initialize ArduCAM
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  
  // Check if OV5642 is detected
  uint8_t vid, pid;
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  
  if ((vid != 0x56) || (pid != 0x42)) {
    Serial.println("Can't find OV5642 module!");
    Serial.print("VID: 0x");
    Serial.print(vid, HEX);
    Serial.print(", PID: 0x");
    Serial.println(pid, HEX);
    while(1);
  } else {
    Serial.println("OV5642 detected.");
  }
  
  // Initialize OV5642 camera
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   // VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);        // Set initial resolution
  
  delay(1000);
  myCAM.clear_fifo_flag();
  
  // Optional: Setup WiFi and web server
  setupWiFi();
  setupWebServer();
  
  Serial.println("Setup complete. Camera ready!");
  Serial.println("Commands:");
  Serial.println("- Type 'capture' to take a photo");
  Serial.println("- Type 'stream' to start streaming mode");
  Serial.println("- Type 'res320' for 320x240 resolution");
  Serial.println("- Type 'res640' for 640x480 resolution");
  Serial.println("- Type 'res1024' for 1024x768 resolution");
  Serial.println("- Type 'res1280' for 1280x960 resolution");
  Serial.println("- Type 'res1600' for 1600x1200 resolution");
  Serial.println("- Type 'res2048' for 2048x1536 resolution");
  Serial.println("- Type 'res2592' for 2592x1944 resolution (5MP)");
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Handle serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "capture") {
      captureImage();
    } else if (command == "stream") {
      streamImages();
    } else if (command == "res320") {
      myCAM.OV5642_set_JPEG_size(OV5642_320x240);
      Serial.println("Resolution set to 320x240");
    } else if (command == "res640") {
      myCAM.OV5642_set_JPEG_size(OV5642_640x480);
      Serial.println("Resolution set to 640x480");
    } else if (command == "res1024") {
      myCAM.OV5642_set_JPEG_size(OV5642_1024x768);
      Serial.println("Resolution set to 1024x768");
    } else if (command == "res1280") {
      myCAM.OV5642_set_JPEG_size(OV5642_1280x960);
      Serial.println("Resolution set to 1280x960");
    } else if (command == "res1600") {
      myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);
      Serial.println("Resolution set to 1600x1200");
    } else if (command == "res2048") {
      myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);
      Serial.println("Resolution set to 2048x1536");
    } else if (command == "res2592") {
      myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
      Serial.println("Resolution set to 2592x1944 (5MP)");
    }
  }
  
  delay(10);
}

void captureImage() {
  Serial.println("Capturing image...");
  
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  
  // Wait for capture to complete
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  
  uint32_t length = myCAM.read_fifo_length();
  Serial.print("Image size: ");
  Serial.print(length);
  Serial.println(" bytes");
  
  if (length >= MAX_FIFO_SIZE) {
    Serial.println("Over size.");
    return;
  }
  
  if (length == 0) {
    Serial.println("Size is 0.");
    return;
  }
  
  // Read and output image data
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  
  Serial.println("--- IMAGE DATA START ---");
  
  uint8_t temp = 0, temp_last = 0;
  length--;
  
  while (length--) {
    temp_last = temp;
    temp = SPI.transfer(0x00);
    
    // Print as hex for debugging, or save to SD card
    if (temp < 16) Serial.print("0");
    Serial.print(temp, HEX);
    Serial.print(" ");
    
    // JPEG end marker
    if ((temp == 0xD9) && (temp_last == 0xFF)) {
      break;
    }
  }
  
  myCAM.CS_HIGH();
  Serial.println("\n--- IMAGE DATA END ---");
  
  myCAM.clear_fifo_flag();
}

void streamImages() {
  Serial.println("Starting stream mode (press any key to stop)...");
  
  while (!Serial.available()) {
    captureImage();
    delay(1000); // 1 second between captures
  }
  
  // Clear the serial buffer
  while (Serial.available()) {
    Serial.read();
  }
  
  Serial.println("Stream stopped.");
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed. Continuing without WiFi.");
  }
}

void setupWebServer() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("Web server started");
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ArduCAM ESP32</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 40px; background-color: #f5f5f5; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 30px; }";
  html += ".btn { display: inline-block; padding: 12px 24px; margin: 10px; background-color: #007bff; color: white; text-decoration: none; border-radius: 5px; transition: background-color 0.3s; }";
  html += ".btn:hover { background-color: #0056b3; }";
  html += ".status { margin: 20px 0; padding: 15px; background-color: #e9f7ef; border-left: 4px solid #27ae60; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>ArduCAM ESP32 Control Panel</h1>";
  html += "<div class='status'>";
  html += "<strong>Status:</strong> Camera Ready<br>";
  html += "<strong>IP Address:</strong> " + WiFi.localIP().toString() + "<br>";
  html += "<strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes";
  html += "</div>";
  html += "<p><a href='/capture' class='btn'>📸 Capture Image</a></p>";
  html += "<p><a href='/status' class='btn'>🔍 Check Status</a></p>";
  html += "<p><small>Tip: The capture endpoint streams JPEG data directly to your browser</small></p>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleCapture() {
  Serial.println("Web capture requested");
  
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  
  uint32_t length = myCAM.read_fifo_length();
  
  if (length >= MAX_FIFO_SIZE || length == 0) {
    server.send(500, "text/plain", "Camera error");
    return;
  }
  
  // Send image directly to browser
  server.setContentLength(length);
  server.send(200, "image/jpeg", "");
  
  WiFiClient client = server.client();
  
  // Check if client is still connected
  if (!client.connected()) {
    myCAM.clear_fifo_flag();
    return;
  }
  
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  
  // Use buffered transfer for better performance
  const size_t bufferSize = 1024; // 1KB buffer
  uint8_t buffer[bufferSize];
  size_t bufferIndex = 0;
  uint32_t bytesRemaining = length;
  
  while (bytesRemaining > 0 && client.connected()) {
    // Fill buffer
    size_t bytesToRead = min(bufferSize - bufferIndex, bytesRemaining);
    
    for (size_t i = 0; i < bytesToRead; i++) {
      buffer[bufferIndex + i] = SPI.transfer(0x00);
    }
    
    bufferIndex += bytesToRead;
    bytesRemaining -= bytesToRead;
    
    // Send buffer when full or when all data is read
    if (bufferIndex == bufferSize || bytesRemaining == 0) {
      size_t bytesWritten = client.write(buffer, bufferIndex);
      
      // Handle partial writes
      if (bytesWritten < bufferIndex) {
        // Move remaining data to beginning of buffer
        memmove(buffer, buffer + bytesWritten, bufferIndex - bytesWritten);
        bufferIndex -= bytesWritten;
      } else {
        bufferIndex = 0;
      }
    }
    
    // Small delay to prevent overwhelming the client
    if (bufferIndex == 0) {
      delay(1);
    }
  }
  
  myCAM.CS_HIGH();
  myCAM.clear_fifo_flag();
  
  if (!client.connected()) {
    Serial.println("Client disconnected during transfer");
  }
}

void handleStatus() {
  String json = "{";
  json += "\"status\": \"online\",";
  json += "\"freeHeap\": " + String(ESP.getFreeHeap()) + ",";
  json += "\"uptime\": " + String(millis()) + ",";
  json += "\"wifiRSSI\": " + String(WiFi.RSSI()) + ",";
  json += "\"ipAddress\": \"" + WiFi.localIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json", json);
}