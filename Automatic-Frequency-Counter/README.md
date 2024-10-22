# Automatic Frequency Counter (AFC)

## Overview

In this project an Automatic Frequency Counter (AFC) was designed using a PIC18F45K50 microcontroller. The AFC operates in two modes:

- **Direct Frequency Counter (DFC)** for frequencies above 100 Hz.
- **Reciprocal Frequency Counter (RFC)** for frequencies below 100 Hz.

The system switches between these modes automatically based on the input signal. It displays the frequency on an 8-digit 7-segment display, and two LEDs indicate whether the frequency is in Hertz (Hz) or Megahertz (MHz).

## System Components

- **Microcontroller**: PIC18F45K50
- **Display**: 8-digit 7-segment display
- **Input**: Frequency signal, push button to start/stop
- **Indication**: LEDs for Hz and MHz modes
- **Oscillator**: 12 MHz external oscillator with PLL x4 (48 MHz system clock)

## Features

- Automatic switching between DFC and RFC based on input frequency.
- Frequency measurements from low Hz to several MHz.
- Display frequency on an 8-digit 7-segment display.
- LEDs indicating the frequency range (Hz or MHz).
- Push-button for start/stop.

## Pseudo Code

Below is the pseudocode used as the foundation for the project:

```python
# Declare and initialise variables

gate_time = 1  # in seconds
cycles_counted = 0
total_time_period = 0
frequency = 0

# DFC: Direct Frequency Counting
def dfc():
    start_timer()
    wait(gate_time)
    stop_timer()
    cycles_counted = count_cycles()
    frequency = cycles_counted / gate_time
    return frequency

# RFC: Reciprocal Frequency Counting
def rfc():
    total_time_period = measure_time_period()
    frequency = 1 / total_time_period
    return frequency

# Automatic switching between DFC and RFC
def switch_between_dfc_and_rfc():
    if frequency < 100:
        frequency = rfc()
    else:
        frequency = dfc()
    return frequency
```

## Configuration

The system begins by configuring various hardware components:

- **Timers**: Timer 1 for counting pulses, Timer 2 for 1-second intervals, Timer 3 for RFC calculations.
- **External Oscillator**: The system uses a 12 MHz external crystal oscillator, multiplied by 4 using the PLL to achieve a 48 MHz clock.

Here is a snippet of the code configuring the external oscillator:

```c
OSCCONbits.SCS = 0b00;        // Use primary oscillator
OSCCON2bits.PLLEN = 1;        // Enable PLL
OSCTUNEbits.SPLLMULT = 0;     // PLL x4 (12MHz * 4 = 48MHz)
```

## Functions

**DFC()** This function counts the number of pulses over a period of 1 second (using Timer 2) and calculates the frequency.

```c 
void DFC() {
    if(t2 == 0) {
        TMR1 = 0;
        t2 = 0;
        T2CONbits.TMR2ON = 1;   // Start Timer 2 (1-second timer)
        T1CONbits.TMR1ON = 1;   // Start Timer 1 (pulse counter)
    }
    if(t2 >= 375) {
        DFCval = TMR1 + (65536 * overflowt1);
        TMR1 = 0;
        overflowt1 = 0;
        t2 = 0;
    }
}
```

**RFC()** This function measures the time between two rising edges of a signal (using Timer 3) and calculates the frequency by taking the reciprocal of the period.

```c
void RFC() {
    if(T3CONbits.TMR3ON == 0) {
        t3 = TMR3;
        RFCval = (4000000 / t3) + overflowt3 * 65536;
        t3 = 0;
        TMR3 = 0;
        overflowt3 = 0;
    }
}
```

## User Interface 

- Push Button: The push button is used to start or stop the frequency measurement. It is connected to an external interrupt pin (RB2).
- 7-segment display: Used to display the measured frequency with a frequency of 72.5 Hz per digit.
- LEDs: Two LEDs indicate the mode (Hz or MHz). The green LED indicates MHz, and the red LED indicates Hz.

## Results and Testing

### RFC (less than 100Hz)

| Frequency (Hz) | Display (Hz) | Error (Hz) | Error (%) |
|----------------|--------------|------------|-----------|
| 1              | 1            | 0          | 0.00      |
| 50             | 50           | 0          | 0.00      |
| 100            | 101          | 1          | 1.00      |

### DFC (more than 100Hz)

| Frequency (MHz) | Display (MHz) | Error (MHz) | Error (%) |
|-----------------|---------------|-------------|-----------|
| 0.001           | 0.001         | 0.000       | 0.00      |
| 0.1             | 0.101         | 0.001       | 1.00      |
| 5               | 5.05          | 0.05        | 1.00      |
