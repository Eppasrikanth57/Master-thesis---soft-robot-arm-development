#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// ---------------------- Pins ----------------------
const int buttonPin = 13;
const int relayPin2 = 5;
const int relayPin1 = 6;

// Motor 1
const int stepPin1 = 30;
const int dirPin1  = 31;

// Motor 2
const int stepPin2 = 32;
const int dirPin2  = 33;

// ---------------------- Sensor Averaging ----------------------
const int MAX_DATA = 10;
int data[MAX_DATA] = {};
int readDataIndex = 0;
bool bufferFull = false;

// ---------------------- Vacuum Control ----------------------
const int threshold = 500;
int smoothedDistance = 1000;  // default far

bool vacuumState = false;        // true if vacuum sequence running
unsigned long vacuum2StartTime = 0;
unsigned long vacuum1LastToggle = 0;
bool vacuum1Active = false;      // true = ON, false = OFF
bool vacuum1Enabled = false;     // true once 7s delay has passed
const unsigned long VACUUM_DELAY = 7000; // 7 sec delay

// ---------------------- Motor Control ----------------------
const int stepsPerRevolution = 1500;
const int totalStepsPerCycle = stepsPerRevolution * 2;
unsigned long stepInterval = 500; // microseconds between steps

bool motorActive = false;
bool isPaused = false;
bool direction = false; // false=ACW, true=CW
unsigned long lastStepTime = 0;
unsigned long pauseStartTime = 0;
unsigned long pauseDuration = 2000;

int stepCount = 0;
int completedCycles = 0;
const int maxCycles = 2;

// ---------------------- Options ----------------------
// Set this to true if you want Motor 2 to rotate opposite to Motor 1
const bool mirroredMotors = false;  

// ---------------------- Setup ----------------------
void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin1, OUTPUT);

  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);

  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);

  // Vacuum OFF initially
  digitalWrite(relayPin2, HIGH);
  digitalWrite(relayPin1, HIGH);
  vacuumState = false;
  vacuum1Active = false;
  vacuum1Enabled = false;

  // Set initial motor directions
  digitalWrite(dirPin1, direction ? HIGH : LOW);
  digitalWrite(dirPin2, mirroredMotors ? (direction ? LOW : HIGH) : (direction ? HIGH : LOW));

  // Init sensor buffer
  for (int i = 0; i < MAX_DATA; i++) data[i] = 1000;

  Serial.println("Starting up...");

  if (!lox.begin()) {
    Serial.println("Failed to initialize VL53L0X!");
    while (1);
  }

  Serial.println("System ready - Press button to start motors");
}

// ---------------------- Sensor ----------------------
int getSmoothedDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  int currentReading = measure.RangeMilliMeter;

  if (measure.RangeStatus <= 2) {
    data[readDataIndex] = currentReading;
    readDataIndex = (readDataIndex + 1) % MAX_DATA;
    if (readDataIndex == 0) bufferFull = true;
  } else {
    return smoothedDistance;
  }

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
  completedCycles = 0;
  direction = false;
  Serial.println("Motor cycles reset");
}

// ---------------------- Loop ----------------------
void loop() {
  // -------- ToF + Vacuum --------
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead >= 50) {
    lastSensorRead = millis();
    int dist = getSmoothedDistance();

    if (dist > 0) {
      smoothedDistance = dist;

      if (smoothedDistance < threshold && vacuumState) {
        // Object detected → turn everything OFF + reset
        digitalWrite(relayPin2, HIGH);
        digitalWrite(relayPin1, HIGH);
        vacuumState = false;
        vacuum1Enabled = false;
        vacuum1Active = false;
        Serial.println("Object detected - Both vacuums OFF, reset");
      } 
      else if (smoothedDistance >= threshold && !vacuumState) {
        // No object → start sequence
        digitalWrite(relayPin2, LOW);   // Vacuum 2 ON immediately
        digitalWrite(relayPin1, HIGH);  // Vacuum 1 OFF initially
        vacuumState = true;
        vacuum2StartTime = millis();
        vacuum1Enabled = false;
        vacuum1Active = false;
        Serial.println("No object - Vacuum 2 ON, Vacuum 1 will start in 7s");
      }
    }
  }

  // -------- Vacuum 1 Control --------
  if (vacuumState) {
    // enable vacuum1 after first 7s delay
    if (!vacuum1Enabled && millis() - vacuum2StartTime >= VACUUM_DELAY) {
      vacuum1Enabled = true;
      vacuum1Active = true;
      digitalWrite(relayPin1, LOW);   // turn ON
      vacuum1LastToggle = millis();
      Serial.println("Vacuum 1 ON (cycling started)");
    }

    // toggle vacuum1 every 7s after enabled
    if (vacuum1Enabled && millis() - vacuum1LastToggle >= VACUUM_DELAY) {
      vacuum1Active = !vacuum1Active;
      digitalWrite(relayPin1, vacuum1Active ? LOW : HIGH);
      vacuum1LastToggle = millis();
      Serial.print("Vacuum 1 toggled -> ");
      Serial.println(vacuum1Active ? "ON" : "OFF");
    }
  }

  // -------- Button Check --------
  static bool lastButtonState = false;
  bool currentButtonState = digitalRead(buttonPin);

  if (currentButtonState && !lastButtonState && !motorActive && !isPaused && completedCycles < maxCycles) {
    motorActive = true;
    stepCount = 0;
    digitalWrite(dirPin1, direction ? HIGH : LOW);
    digitalWrite(dirPin2, mirroredMotors ? (direction ? LOW : HIGH) : (direction ? HIGH : LOW));
    Serial.print("Motors started - Direction: ");
    Serial.println(direction ? "CW" : "ACW");
  }
  lastButtonState = currentButtonState;

  // -------- Motor Stepping --------
  if (motorActive) {
    unsigned long currentMicros = micros();
    if (currentMicros - lastStepTime >= stepInterval) {
      lastStepTime = currentMicros;

      // Step pulse both motors
      digitalWrite(stepPin1, HIGH);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(20);  // longer pulse for reliability
      digitalWrite(stepPin1, LOW);
      digitalWrite(stepPin2, LOW);

      stepCount++;

      if (stepCount % 800 == 0) {
        Serial.print("Steps: ");
        Serial.print(stepCount);
        Serial.print("/");
        Serial.println(totalStepsPerCycle);
      }

      if (stepCount >= totalStepsPerCycle) {
        motorActive = false;
        isPaused = true;
        pauseStartTime = millis();
        Serial.println("Motor sequence completed, pausing");
      }
    }
  }

  // -------- Pause Between Cycles --------
  if (isPaused && (millis() - pauseStartTime >= pauseDuration)) {
    isPaused = false;
    completedCycles++;

    if (completedCycles < maxCycles) {
      direction = !direction;
      motorActive = true;
      stepCount = 0;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      digitalWrite(dirPin2, mirroredMotors ? (direction ? LOW : HIGH) : (direction ? HIGH : LOW));
      Serial.print("Starting cycle ");
      Serial.print(completedCycles + 1);
      Serial.print(" - Direction: ");
      Serial.println(direction ? "CW" : "ACW");
    } else {
      Serial.println("All motor sequences completed");
      resetMotorCycles();
      Serial.println("Motors auto-reset. Ready for next button press.");
    }
  }
}
