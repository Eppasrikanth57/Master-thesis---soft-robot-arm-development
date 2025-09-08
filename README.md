# Master-thesis---soft-robot-arm-development
Design and Prototyping of a Soft Robot Arm for Safe and Contact-Rich Human-Robot Collaboration
This repository contains the embedded control system architecture and code for the master's thesis project, "Design and Prototyping of a Soft Robot Arm for Safe and Contact-Rich Human-Robot Collaboration." The work focuses on developing a soft robot for Industry 5.0 applications, with a strong emphasis on safety and direct human-robot interaction.

Project Overview
The core objective of this thesis was to design, prototype, and test a pneumatic soft robot arm capable of safe collaboration with humans. The project involved:

Mechanical Design: Two prototypes were created and optimized using SolidWorks.

Additive Manufacturing: Key components of the robot were fabricated using 3D printing.

Embedded Systems Integration: An Arduino Mega 2560 and ESP32 was used to develop the control system, integrating various sensors and actuators.

Embedded Control System
The embedded control system, developed on the Arduino Mega 2560 platform, is the brain of the robot arm. It is responsible for:

Human Detection: Using a Time-of-Flight (ToF) sensor to detect the presence of humans or objects within the robot's workspace.

Impact Analysis: Utilizing a Force-Sensitive Resistor (FSR) sensor to analyze contact forces, ensuring safe interaction.

Motor Control: Actuating stepper motors for precise control of the arm's movement.

Pneumatic Control: Managing the vacuum system via relays to control the soft arm's inflation and deflation.

Communication Protocols: Handling serial communication for debugging, data logging, and parameter adjustments.

File Descriptions
This repository includes the following Arduino sketch files:

FSR.ino: This is a standalone sketch for testing the Force-Sensitive Resistor (FSR) sensor. It reads analog data from the FSR pin and prints the raw force value to the serial monitor. This file is crucial for calibrating the sensor and understanding its response to different levels of pressure.

Strain_guage.ino: This sketch is designed to measure the tension of a cable using a foil-type strain gauge connected to a Wheatstone bridge. It reads the analog output from the bridge, converts the value to voltage, and prints the result to the serial monitor for analysis.

Pneubot_2_Vacuum_controls_500mm_Range.ino: This file contains the main control logic for the robot arm. It integrates the Time-of-Flight (ToF) sensor to control the vacuum system. When an object is detected within a 500mm range, the vacuum is turned off, causing the soft arm to retract. It also includes the control logic for a stepper motor, which cycles between two directions.

Pneubot_2_Vacuum_controls__delay5sec_500mm_range.ino: This is an improved version of the previous file, adding a 5-second delay to the vacuum control sequence. This means the second vacuum is only turned on after a brief pause, providing more nuanced control over the arm's state. This file is the final and most optimized version of the main control code.

Getting Started
To use this code, you will need:

An Arduino Mega 2560 board.

A VL53L0X Time-of-Flight (ToF) sensor.

A Force-Sensitive Resistor (FSR) sensor.

A foil-type strain gauge and a Wheatstone bridge.

Relay modules and a vacuum pump system.

A stepper motor and a corresponding motor driver.

An understanding of the Arduino IDE and basic C++ programming.

For more detailed information on the project's design and experimental results, please refer to the complete thesis document.

Acknowledgements
Adafruit Industries: For the VL53L0X library.
