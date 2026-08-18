# DC Motor Regulator with BEMF Feedback

This project implements a DC motor speed regulator using Back-Electromotive Force (BEMF) feedback and a PID control algorithm. The system is built around an STM32F103C8 microcontroller (Blue Pill) and utilizes SEGGER Real-Time Transfer (RTT) for efficient debugging and logging.

## Features

*   **BEMF Sensing:** Measures the motor's BEMF to determine its actual rotational speed without external sensors.
*   **PID Control:** Employs a Proportional-Integral-Derivative (PID) controller to maintain the desired motor speed.
*   **Configurable Parameters:** PID coefficients (Ki, Kd) and target speed are adjustable via analog potentiometers. Kp is fixed.
*   **Smooth Start:** Implements a smooth ramp-up for the target speed to prevent sudden motor jerks.
*   **Voltage Monitoring:** Monitors the motor supply voltage (VMOT) and prevents operation if it's too low.
*   **SEGGER RTT Logging:** Provides real-time debugging output to the host PC via J-Link, offering insights into control parameters, BEMF, and PWM values.

## Hardware

*   **Microcontroller:** STM32F103C8 (Blue Pill)
*   **DC Motor:** A brushed DC motor.
*   **Motor Driver:** An H-bridge motor driver (e.g., L298N, DRV8871, etc.) capable of driving the DC motor.
*   **Voltage Dividers:** For BEMF and VMOT sensing.
*   **Potentiometers:** Three potentiometers for setting target speed, Ki, and Kd.
*   **J-Link Debugger:** For programming and SEGGER RTT communication.

## Project Structure

The project is organized into two main parts:

*   `Firmware/`: Contains the embedded software developed using PlatformIO.
*   `Altium/`: Contains the hardware design files (schematics, PCB layout) created with Altium Designer.

*   **`Docs/`**: Contains additional documentation, hardware connection guides, and project photos.

## Getting Started

To get started with this project, please refer to the specific `README.md` files in each subdirectory:

*   For firmware development and deployment: `Firmware/README.md`
*   For hardware design and manufacturing: `Altium/README.md`
*   For detailed hardware connections and other documentation: `Docs/README.md`