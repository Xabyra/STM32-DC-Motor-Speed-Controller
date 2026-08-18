# Firmware for DC Motor Regulator (PlatformIO)

This directory contains the firmware for the DC Motor Regulator project, developed using PlatformIO with the Arduino framework for the STM32F103C8 microcontroller.

## Project Contents

## Project Overview

The firmware implements the core control logic for regulating a DC motor's speed based on BEMF feedback. It includes:
*   Analog-to-Digital Converter (ADC) readings for BEMF, motor supply voltage (VMOT), target speed, and PID coefficients.
*   Exponential Moving Average (EMA) filters for smoothing sensor readings.
*   A custom PID controller with anti-windup and feedforward capabilities.
*   SEGGER RTT integration for real-time debugging output.

## Requirements

*   **PlatformIO IDE:** Recommended to use the PlatformIO extension for VS Code.
*   **J-Link Debugger:** Required for uploading firmware and utilizing SEGGER RTT.
*   **STM32F103C8 Board:** (e.g., Blue Pill) connected to the J-Link.

## Installation and Setup

1.  **Install PlatformIO:** If you haven't already, install VS Code and the PlatformIO IDE extension.
2.  **Clone Repository:** Clone this project to your local machine.
3.  **Open Project:** Open the `Firmware` directory in VS Code. PlatformIO should automatically detect the project.
4.  **Connect Hardware:**
    *   Connect your STM32F103C8 board to your J-Link debugger (SWD interface).
    *   Ensure the J-Link is connected to your computer.
    *   Refer to the `Docs/Hardware_Connections.md` for detailed wiring instructions.

## Building and Uploading

The `platformio.ini` file is configured for the `genericSTM32F103C8` board and uses `jlink` for upload and debugging.

To build and upload the firmware:
1.  In VS Code, open the PlatformIO sidebar.
2.  Under the "PROJECT TASKS" section for `genericSTM32F103C8`, click on "Build" to compile the code.
3.  After a successful build, click on "Upload" to flash the firmware to your STM32F103C8 board via J-Link.

## SEGGER RTT Debugging

The firmware uses SEGGER RTT for logging and debugging information. This allows you to view real-time output from the microcontroller without halting its execution.

1.  **Start J-Link RTT Viewer:** Open the J-Link RTT Viewer application on your host PC.
2.  **Configure RTT Viewer:**
    *   Select your J-Link device.
    *   Choose "SWD" as the target interface.
    *   Specify the target device (e.g., `STM32F103C8`).
    *   Set the target interface speed (e.g., 4000 kHz).
    *   Click "OK" to connect.
3.  **View Output:** Once connected, you should see the real-time log messages from the microcontroller, including BEMF, target speed, PWM, and PID coefficients.

Example output:
```
RUN | Tar:15000 mV | BEMF:14800 mV | FF:127 | PWM:128/255 | Ki: 50/1000 | Kd: 10/1000
```

## Code Structure (`main.cpp`)

*   **Configuration Defines:** `MAX_TARGET_EMF`, `PID_DIVIDER`, pin assignments, etc.
*   **`IntEMA` Class:** A simple Exponential Moving Average filter for smoothing analog readings.
*   **`PIDreg` Class:** Implements the PID control logic.
*   **`setup()`:** Initializes SEGGER RTT, ADC, PWM, and EMA filters.
*   **`loop()`:** Continuously calls the `control()` function.
*   **`control()`:**
    *   Disables PWM temporarily to measure BEMF.
    *   Reads `VMOT_PIN` and `BEMF_PIN` via voltage dividers.
    *   Filters all analog inputs using EMA.
    *   Calculates PID coefficients from potentiometer readings.
    *   Implements safety checks for `VMOT` and `Target EMF`.
    *   Applies a smooth start to the target speed.
    *   Calculates PWM output using the PID controller and feedforward.
    *   Logs critical data via SEGGER RTT.

## Software Required

*   **Altium Designer:** You will need Altium Designer (version used for creation or compatible) to open, view, and modify these files.

## Opening the Project

1.  **Install Altium Designer:** Ensure you have Altium Designer installed on your system.
2.  **Clone Repository:** Clone this project to your local machine.
3.  **Open Project File:** Navigate to this `Altium/` directory and open the `.PrjPcb` file (e.g., `DCMotorRegulator.PrjPcb`) using Altium Designer.

## Key Hardware Components

The design typically includes:
*   **STM32F103C8 Microcontroller:** The core processing unit.
*   **Motor Driver:** An H-bridge circuit to control the DC motor (e.g., based on MOSFETs or an integrated driver IC).
*   **Voltage Dividers:** Resistor networks for scaling down the motor voltage (VMOT) and BEMF for ADC input.
*   **Potentiometers:** For user input (target speed, PID tuning).
*   **Power Management:** Voltage regulators to supply the microcontroller and other components.
*   **Connectors:** For motor, power, J-Link, and potentiometers.

## Manufacturing

Once the design is finalized, you can generate manufacturing outputs (Gerber files, NC Drill files, BOM, Pick & Place files) directly from Altium Designer to send to a PCB fabrication house and assembly service.