void setup() {
  delay(3000);

  // Initialize Serial port
  Serial.begin(115200);
  Serial.println("setup");

  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH);
  delay(1000);
  Serial.println("on");
  digitalWrite(13, LOW);
  delay(1000);
  Serial.println("off");
}
