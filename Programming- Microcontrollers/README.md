# Digital Voltmeter Project

## Overview

This project is a digital voltmeter designed and implemented using a PIC18F45K50 microcontroller. The voltmeter reads analog input voltages, processes them using the microcontroller's ADC (Analog-to-Digital Converter), and displays the voltage readings on a 7-segment display.

## System Components

- **Microcontroller**: PIC18F45K50
- **Display**: 4-digit 7-segment display
- **Input Voltage**: Variable voltage supplied through a trimmer
- **Filter**: Second-order low-pass filter to ensure stable voltage readings
- **Firmware**: Programmed in C using MPLAB X IDE

## Features

- Analog-to-digital conversion with a 10-bit resolution
- Accurate voltage display with up to 3 decimal places
- ADC input pin with a trimmer to provide variable voltage
- Low-pass filter to reduce noise in the input voltage
- 7-segment display to show the converted voltage
- Internal Fixed Voltage Reference (FVR) used to enhance the accuracy of the ADC

## How It Works

1. **Hardware**: The circuit is implemented on a breadboard with a trimmer connected to the analog input pin of the PIC18F45K50. The microcontroller reads the input voltage and converts it into a digital value using the ADC.
   
2. **Software**: The firmware configures the necessary registers for the ADC, timers, and interrupts. It continuously reads the voltage and displays the value on a 7-segment display.

3. **Filtering**: A second-order low-pass filter is used to remove high-frequency components and noise, ensuring stable voltage readings.

4. **Error Handling**: The system was calibrated to measure errors. The maximum error percentage is about 1.6%, and future improvements could reduce this further.

## Circuit Design

- The analog voltage is provided via a trimmer connected to pin RA0 of the PIC.
- The voltage is filtered using a low-pass filter to ensure stability.
- The PIC’s ADC converts the analog voltage, which is then displayed on the 7-segment display using the predefined 7-segment patterns.

## Firmware Description

- **Pseudocode**: Before implementation, the system's logic was outlined in pseudocode to ensure proper flow and functionality.
- **ISR (Interrupt Service Routine)**: High-priority interrupts handle ADC conversions, where the result is processed and the voltage value is updated on the display.
- **NumDisplay**: A function is used to convert the voltage value into its corresponding 7-segment display format, lighting up the appropriate segments on the display.

## Accuracy and Calibration

The voltmeter was tested with various input voltages, comparing the displayed values to a calibrated voltmeter. The average error across the tests was minimal (approximately 1.6%). Suggested improvements for reducing this error include transitioning from a breadboard to a PCB and improving the analog source impedance.

### Readings taken to determine the error characteristic:

| Vin (V) | Displayed Value (V) | Error (V) | Error (%) |
|---------|---------------------|-----------|-----------|
| 0.25    | 0.257               | 0.007     | 2.80      |
| 1.00    | 1.018               | 0.018     | 1.80      |
| 2.00    | 2.031               | 0.031     | 1.60      |
| 4.00    | 4.075               | 0.075     | 1.88      |


