# Armduino

## Flashing the Firmware

Download the code for Arduino, extract and open with the Arduino IDE. Before attempting to flash the [Seeed Studio XIAO SAMD21 used to be named as Seeeduino XIAO board](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html), you will need to add the board support to the IDE with the following instructions. Once complete, verify the correct board is chosen, port is selected and upload.

https://wiki.seeedstudio.com/Seeed_Arduino_Boards/

## How to Use
When powered on and connected to a DJI 03 air unit (or Caddx Vista) it will show the voltage on the goggles OSD once it is ready to arm. A PWM receiver programmed to output a high signal to D10 will send out the arm signal to the DJI VTX. If record on arm is configured in the goggles, you will see the recording begin. The bitrate should jump from ~7mbps in standby to ~50mbps indicating full power is achieved.

## Wiring Guide (DO NOT FEED VBAT TO VCC)

![Seeeduino_Diagram](https://github.com/Saiteik/Armduino/blob/main/Seeduino_XAIO_SAMD21.png?raw=true)

* Option 1 (Arm only): 5v and ground from BEC, TX and RX to DJI VTX, PWM signal from receiver to D10.
* Option 2 (Arm + Voltage): 5v and ground from BEC, TX and RX to DJI VTX, PWM signal from receiver to D10. Voltage divided by resistors from battery voltage to VCC.

Use this [Voltage Divider](https://ohmslawcalculator.com/voltage-divider-calculator) site to assist with calculating the correct resistor choice for your battery application. For example, for a 6s lipo battery, I am using a 31k and 4.7k pair of resistors. The purpose of the resistor choice is to ensure that a full battery voltage equals 3.3 volts or less.

## Calibration

The code will need to be calibrated to account for deviations in the resistors or use of different lipo battery cell count (default for 6s). Using the Arduino IDE, find the following definitions and update their respective values.

* VCC_SCALE - Adjust it until the readout in the goggles match the voltage of the battery.
* ValueR1 - R1 resistor value, ex: 33k ohm resistor equals "33000.0"
* ValueR2 - R2 resistor value, ex: 4.7k ohm resistor equals "4700.0"

There are 2 presets provided that are pre-calibrated for 4s or 6s.

4S Lipo Battery Voltage
VCC 1010
R1 22000.0    - 22K   Resistor
R2 5100.0     - 5.1K  Resistor

6S Lipo Battery Voltage
VCC 1015
R1 33000.0    - 33K   Resistor
R2 4700.0     - 4.7K  Resistor