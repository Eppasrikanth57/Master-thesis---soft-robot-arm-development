// Constants
const int analogPin = A0;            // A0 is connected to Vout (+) of the Wheatstone Bridge
const float referenceVoltage = 5.0;  // Using the 3.3V reference

void setup() {
  Serial.begin(9600);          // Start the serial communication
  delay(1000);                 // Wait for stabilization
  Serial.println("Strain Gauge Voltage Measurement Initialized...");
  Serial.println("Voltage (V)");
}

void loop() {
  // Read the analog value from A0
  int sensorValue = analogRead(analogPin);

  // Convert the analog value to voltage
  float voltage = (sensorValue * referenceVoltage) / 1023.0;

  // Display the voltage
  Serial.println(voltage, 6);   // Voltage with 6 decimal places

  // Wait a bit before the next reading
  delay(500);
}

