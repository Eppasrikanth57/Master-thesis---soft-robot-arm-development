int fsrPin = A0;  // FSR connected to A0
int fsrValue = 0;  // Variable to store the FSR reading

void setup() {
  Serial.begin(9600);  // Initialize serial communication
}

void loop() {
  fsrValue = analogRead(fsrPin);  // Read the FSR value
  Serial.println(fsrValue);  // Print the value to the serial monitor
  delay(500);  // Delay to make the readings more visible
}

