#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>

#define OV5642_MINI_5MP
#include <ArduCAM.h>

const char* ssid = "a-tp";
const char* password = "opop9090";
const char* SERVER_URL = "http://localhost:8080";

#define CS_PIN 10  // Pin for Chip Select

ArduCAM myCAM(OV5642, CS_PIN);  // Create an instance of the ArduCAM class

SPIClass mySPI(HSPI);  // Use VSPI (or HSPI depending on your pin configuration)

void setup() {
  uint8_t vid, pid;
  uint8_t temp;

  Serial.begin(115200);
  delay(1000);  // Wait for the serial monitor to open
  Serial.println("ArduCAM OV5642 Camera Setup");

  // Try slowing down SPI
  SPI.setFrequency(1000000);  // Slow to 1MHz

  // Try changing SPI mode
  SPI.setDataMode(SPI_MODE0);  // Try different modes if this doesn't work

  // Initialize SPI
  pinMode(CS_PIN, OUTPUT);

  // Add more delay after CS transitions
  digitalWrite(CS_PIN, LOW);
  delay(1);  // Short delay after CS goes LOW
  // SPI operations
  delay(1);  // Short delay before CS goes HIGH
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();

  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);  // Performing a reset sequence
  delay(100);
  myCAM.write_reg(0x07, 0x00);  // returns it to normal operation
  delay(100);

  // Check if the ArduCAM SPI bus is working
  while (1) {
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);

    if (temp == 0x55) {
      Serial.println("SPI interface is working");
      break;
    } else {
      Serial.println("SPI interface error!");
      delay(1000);
    }
  }

  delay(2000);

  // Check if the camera module is connected properly
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

  Serial.println("Check if the camera is working.");

  if ((vid != 0x56) || (pid != 0x42)) {
    Serial.println(F("Can't find OV5642 module!"));
    // Print the actual values you're getting
    Serial.print("Camera ID - VID: 0x");
    Serial.print(vid, HEX);
    Serial.print(", PID: 0x");
    Serial.println(pid, HEX);
  } else {
    Serial.println(F("OV5642 detected."));
  }

  // Initialize the camera
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);

  Serial.println("Camera Initialized!");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // Print the connected IP address
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Now, make an HTTP request to check internet connectivity
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://httpbin.org/ip");  // URL to make a GET request
    int httpCode = http.GET();            // Send GET request

    if (httpCode > 0) {
      // Print the HTTP response
      String payload = http.getString();
      Serial.println("HTTP Response code: " + String(httpCode));
      Serial.println("Response: " + payload);
    } else {
      Serial.println("Error in HTTP request");
    }
    http.end();  // End the HTTP request
  }
}

void loop() {
  captureAndSendImage();
  delay(10000);  // Take a picture every 10 seconds (adjust as needed)
}

void captureAndSendImage() {
  Serial.println("Capturing image...");
  myCAM.start_capture();

  // // Wait for image to be captured
  // while (!myCAM.isCaptured()) {
  //   delay(100);
  // }

  // Wait for capture done flag
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    delay(100);
  }
  Serial.println("Image captured!");

  // Get the JPEG image size
  uint32_t imageSize = myCAM.read_fifo_length();
  if (imageSize == 0) {
    Serial.println("Error: No image data");
    return;
  }

  // Create a buffer to hold the image
  uint8_t* imgBuffer = (uint8_t*)malloc(imageSize);
  if (imgBuffer == NULL) {
    Serial.println("Error: Memory allocation failed");
    return;
  }

  // Read image data into buffer
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  for (uint32_t i = 0; i < imageSize; i++) {
    imgBuffer[i] = SPI.transfer(0x00);
  }
  myCAM.CS_HIGH();

  // Send the image to the server
  sendImageToServer(imgBuffer, imageSize);

  // Free the allocated memory
  free(imgBuffer);
}

/*
void sendImageToServer(uint8_t* image, uint32_t imageSize) {
  HTTPClient http;
  http.begin(SERVER_URL);

  // Set headers for multipart/form-data (image upload)
  http.addHeader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

  // Build the HTTP POST body
  String body = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
  body += "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n";
  body += "Content-Type: image/jpeg\r\n\r\n";

  // Send the first part of the body (headers + part start)
  http.addHeader("Content-Length", String(body.length() + imageSize + 4));  // Total length of the body
  int httpResponseCode = http.POST(body);

  // Send the image data
  http.write(image, imageSize);

  // End the multipart form data
  String footer = "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
  http.write((uint8_t*)footer.c_str(), footer.length());

  // Check server response
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server Response: " + response);
  } else {
    Serial.println("Error in HTTP request");
  }

  http.end();
}
*/

void sendImageToServer(uint8_t* image, uint32_t imageSize) {
  HTTPClient http;
  http.begin(SERVER_URL);

  // Set headers for multipart/form-data (image upload)
  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  // Build the HTTP POST body with multipart form-data headers
  String body = "--" + boundary + "\r\n";
  body += "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n";
  body += "Content-Type: image/jpeg\r\n\r\n";

  // Send the headers + body start
  int contentLength = body.length() + imageSize + 4 + boundary.length() + 6;  // Calculate total body length
  http.addHeader("Content-Length", String(contentLength));

  // Begin the HTTP POST request
  WiFiClient* stream = http.getStreamPtr();
  stream->print(body);  // Send headers + body start

  // Send the image data
  for (uint32_t i = 0; i < imageSize; i++) {
    stream->write(image[i]);
  }

  // End the multipart form data
  String footer = "\r\n--" + boundary + "--\r\n";
  stream->print(footer);

  // Send the HTTP request
  int httpResponseCode = http.POST("");

  // Check server response
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Server Response: " + response);
  } else {
    Serial.println("Error in HTTP request: " + String(httpResponseCode));
  }

  http.end();
}
