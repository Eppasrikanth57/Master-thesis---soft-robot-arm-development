#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Pins
const int buttonPin = 13;
const int relayPin2 = 5;
const int stepPin1 = 30;
const int dirPin1 = 31;

// Sensor averaging
const int MAX_DATA = 10;
int data[MAX_DATA] = {};
int readDataIndex = 0;
bool bufferFull = false;

// Relay control
const int threshold = 500;
int smoothedDistance = 1000;  // default far

// Variable to track vacuum state
bool vacuumState = true; // vacuum should start as ON

// Motor control
const int stepsPerRevolution = 1500;
const int totalStepsPerCycle = stepsPerRevolution * 2; // 5 rotations
unsigned long stepInterval = 500; // microseconds between steps (adjust for speed)

bool motorActive = false;
bool isPaused = false;
bool direction = false; // Start with ACW
unsigned long lastStepTime = 0;
unsigned long pauseStartTime = 0;
unsigned long pauseDuration = 2000;

int stepCount = 0;
int completedCycles = 0;
const int maxCycles = 2; // Two cycles to do ACW then CW

// Setup
void setup() {
  Serial.begin(115200);
  
  pinMode(buttonPin, INPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  
  // Vacuum ON initially
  digitalWrite(relayPin2, LOW);
  vacuumState = true;

  // Set initial motor direction (LOW = ACW, HIGH = CW)
  digitalWrite(dirPin1, direction ? HIGH : LOW);
  
  // Initialize sensor data array
  for (int i = 0; i < MAX_DATA; i++) {
    data[i] = 1000;
  }
  
  Serial.println("Starting up...");
  
  if (!lox.begin()) {
    Serial.println("Failed to initialize VL53L0X!");
    while (1);
  }
  
  Serial.println("System ready - Press button to start motor");
}

// Read sensor and average
int getSmoothedDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  int currentReading = measure.RangeMilliMeter;

  // Accept statuses 0 (good), 1-2 (valid but less ideal)
  if (measure.RangeStatus <= 2) {
    data[readDataIndex] = currentReading;
    readDataIndex = (readDataIndex + 1) % MAX_DATA;
    if (readDataIndex == 0) bufferFull = true;
  } else {
    // Don't include bad readings in average
    return smoothedDistance;
  }

  // Compute average excluding invalid data
  int sum = 0, count = 0;
  int loopLimit = bufferFull ? MAX_DATA : readDataIndex;
  for (int i = 0; i < loopLimit; i++) {
    if (data[i] > 0) {
      sum += data[i];
      count++;
    }
  }

  int average = (count > 0) ? sum / count : smoothedDistance;

  Serial.print("Raw Distance: ");
  Serial.print(currentReading);
  Serial.print(" | Smoothed: ");
  Serial.println(average);

  return average;
}

void resetMotorCycles() {
    completedCycles = 0;  // Reset the number of completed cycles
    direction = false;    // Reset direction to default (ACW)
    Serial.println("Motor cycles reset");
}

void loop() {
  // Handle ToF sensor and vacuum control
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= 50) {
    lastSensorRead = millis();
    int dist = getSmoothedDistance();
    
    if (dist > 0) {
      smoothedDistance = dist;
      
      // Object detected, turn vacuum off
      if (smoothedDistance < threshold && vacuumState) {
        digitalWrite(relayPin2, HIGH);
        vacuumState = false;
        Serial.println("Object detected - Vacuum OFF");
      } 
      // No object, turn vacuum on
      else if (smoothedDistance >= threshold && !vacuumState) {
        digitalWrite(relayPin2, LOW);
        vacuumState = true;
        Serial.println("No object - Vacuum ON");
      }
    }
  }

  // Check button press to start motor sequence
  static bool lastButtonState = false;
  bool currentButtonState = digitalRead(buttonPin);
  
  if (currentButtonState && !lastButtonState && !motorActive && !isPaused && completedCycles < maxCycles) {
    // Button pressed, start motor
    motorActive = true;
    stepCount = 0;
    digitalWrite(dirPin1, direction ? HIGH : LOW);
    Serial.print("Motor started - Direction: ");
    Serial.println(direction ? "CW" : "ACW");
  }
  lastButtonState = currentButtonState;

  // Motor stepping logic - simplified for reliability
  if (motorActive) {
    unsigned long currentMicros = micros();
    if (currentMicros - lastStepTime >= stepInterval) {
      lastStepTime = currentMicros;
      
      // Generate step pulse
      digitalWrite(stepPin1, HIGH);
      //delayMicroseconds(5); // Ensure pulse is seen by driver
      // Replace delayMicroseconds(5) with micros() implementation
      unsigned long pulseStartTime = micros();
      while (micros() - pulseStartTime < 5) {
        // Empty loop to wait for 5 microseconds
      }
      digitalWrite(stepPin1, LOW);
      
      stepCount++;
      
      // Progress reporting
      if (stepCount % 800 == 0) {
        Serial.print("Steps: ");
        Serial.print(stepCount);
        Serial.print("/");
        Serial.println(totalStepsPerCycle);
      }

      // Check if cycle complete
      if (stepCount >= totalStepsPerCycle) {
        motorActive = false;
        isPaused = true;
        pauseStartTime = millis();
        Serial.println("Motor sequence completed, pausing");
      }
    }
  }

  // Handle pausing between cycles
  if (isPaused && (millis() - pauseStartTime >= pauseDuration)) {
    isPaused = false;
    completedCycles++;
    
    if (completedCycles < maxCycles) {
      // Start next cycle
      direction = !direction;  // Reverse direction
      motorActive = true;
      stepCount = 0;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      Serial.print("Starting cycle ");
      Serial.print(completedCycles + 1);
      Serial.print(" - Direction: ");
      Serial.println(direction ? "CW" : "ACW");
    } else {
      Serial.println("All motor sequences completed");
  
  
    
    // Auto-reset after completion (add these lines)
    //delay(1000);  // Wait 1 seconds before resetting
    resetMotorCycles();  // Reset cycles back to zero
    Serial.println("Motor cycles auto-reset. Ready for next button press.");
  }
}
}

