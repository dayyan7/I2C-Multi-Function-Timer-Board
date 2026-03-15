# I2C Multi-Function Timer Board

A custom-designed embedded timer built on the Arduino Nano ESP32, featuring a physical 
potentiometer, tactile buttons, a 0.96" OLED display, LED signaling, and an audible 
buzzer alert. Developed through a full hardware design cycle — schematic capture, custom 
PCB layout, and manufacturing through JLCPCB — using KiCad.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware & Components](#hardware--components)
- [How It Works](#how-it-works)
- [Schematic Design](#schematic-design)
- [PCB Design](#pcb-design)
- [Power Management & Audio](#power-management--audio)
- [Usage](#usage)
- [Design Philosophy](#design-philosophy)
- [Images](#images)
- [Physical Final Product](#physical-final-product)

---

## Overview

The I2C Multi-Function Timer Board operates as both a real-time digital clock and a 
customizable countdown timer. In its idle state, the OLED displays the current date and 
time. Once a countdown is set, the display transitions to the active timer interface and 
provides live feedback throughout the session.

The project was built through a complete development cycle: schematic design in KiCad, 
custom PCB layout, fabrication by JLCPCB, and firmware written in C++ using the Arduino 
IDE. Every component is intentionally selected and placed to serve a specific purpose.

---

## Features

- Real-time clock and date display on a 0.96" OLED
- Potentiometer for fast, analog minute selection
- Dedicated pushbutton for fine second increments
- Start and Stop buttons for full timer control
- Green and Red LED visual confirmation on Start and Stop
- Dual white power indicator LEDs
- PWM-driven buzzer alert on countdown completion
- Fully custom two-layer PCB with a ground pour on the back

---

## Hardware & Components

The board is built around the **Arduino Nano ESP32**, which handles all timing logic, 
I2C communication with the OLED, and GPIO control for every input and output on the board.

### Component Reference

| Reference | Component       | Value | Function                              |
|-----------|-----------------|-------|---------------------------------------|
| A1        | Arduino Nano ESP32 | —  | Microcontroller                       |
| J1        | OLED Connector  | 4-pin | I2C display (VCC / GND / SCL / SDA)   |
| D1        | LED             | Green | Flashes briefly when Start is pressed |
| D2        | LED             | Red   | Flashes briefly when Stop is pressed  |
| D3        | LED             | White | Power indicator, top-left corner      |
| D4        | LED             | White | Power indicator, top-right corner     |
| BZ1       | Buzzer          | —     | Audible alert on countdown completion |
| SW1       | Tactile Button  | —     | Increment seconds                     |
| SW2       | Tactile Button  | —     | Start timer                           |
| SW3       | Tactile Button  | —     | Stop / End timer                      |
| RV1       | Potentiometer   | —     | Set minutes (analog input)            |
| C1        | Capacitor       | 0.1µF | Decoupling / noise filtering          |
| C2        | Capacitor       | 10µF  | Buzzer current reservoir              |
| Q1 – Q5   | NPN Transistor  | —     | LED and buzzer switching              |
| R1 – R4   | Resistor        | 220Ω  | LED current limiting                  |
| R5 – R9   | Resistor        | 1kΩ   | Transistor base resistors             |

### User Controls

**RV1 — Minutes Potentiometer:** Analog input for coarse minute selection. Turning the 
knob maps directly to a minute value, making it much faster to set longer countdowns 
than using repeated button presses.

**SW1 — Seconds Button:** Increments the second value for fine-tuned timer adjustments.

**SW2 — Start:** Initiates the countdown. Triggers a brief green LED flash as confirmation.

**SW3 — Stop / End:** Halts the countdown at any point. Triggers a brief red LED flash 
as a stop signal.

### LED Indicators

**D1 (Green):** Brief flash when the timer is started.

**D2 (Red):** Brief flash when the timer is stopped.

**D3 & D4 (White):** Positioned in the top two corners flanking the OLED display. These 
remain on as long as the board is receiving 3.3V, serving as power indicators.

---

## How It Works

The firmware runs two core states: **idle (clock mode)** and **active (timer mode)**.

On power-up, the board enters idle mode. The OLED pulls the current date and time and 
displays it as a running clock. Nothing else is active until the user begins setting a 
timer.

When the user adjusts **RV1**, the analog input is read and mapped to a minute value. 
**SW1** handles any fine second adjustments on top of that. Once the desired time is set, 
pressing **SW2** transitions the board from idle to timer mode, displays the countdown on 
the OLED, and flashes **D1** as confirmation.

During the countdown, the display updates in real time. Pressing **SW3** at any point 
exits timer mode, flashes **D2**, and returns the board to idle. If the countdown reaches 
zero without interruption, the Arduino sends a PWM signal to **Q5**, which drives **BZ1** 
to produce an audible alert tone.

The OLED communicates with the Arduino over **I2C** using the SDA and SCL lines. All 
LED outputs are switched through NPN transistors rather than driven directly from GPIO 
pins, which keeps the current load off the microcontroller entirely.

---

## Schematic Design

The schematic was created in *KiCad 9.0.7*. Each subsystem is logically separated: 
LED drive circuitry on the left, the Arduino Nano ESP32 at the center, the buzzer 
circuit on the right, and passive inputs (buttons, potentiometer, OLED) routed into 
their respective GPIO and analog pins.

**LED Drive Circuit:** Each LED is switched through an NPN transistor (Q1–Q4) rather 
than driven directly from a GPIO pin. A 220Ω resistor in series with each LED limits 
current, and a 1kΩ base resistor on each transistor sets the switching threshold. This 
keeps current draw off the microcontroller and protects the output pins from overdrive.

**Buzzer Circuit:** BZ1 is switched by Q5 with R5 as the base resistor. The Arduino 
sends a PWM signal to Q5's base, which switches the buzzer on to produce the alert 
tone. C2 (10µF) sits across the buzzer supply to handle inrush current during 
activation and prevent voltage sag on the 3.3V rail.

**Input Circuitry:** SW1, SW2, and SW3 are tactile switches with one leg tied to GND, 
read as digital inputs on the Arduino. RV1 is wired between 3.3V and GND with its 
wiper feeding into an analog input pin for minute selection. The OLED connects over 
I2C through the SDA/A4 and SCL/A5 pins.

**Decoupling:** C1 (0.1µF) is placed close to the Arduino's 3.3V supply pin to filter 
high-frequency switching noise and keep the supply rail stable.

<img width="542" height="390" alt="Image" src="https://github.com/user-attachments/assets/751d345d-9680-4d3d-8154-dcac6ccd7f75" />

---

## PCB Design

The PCB was designed in *KiCad* and manufactured by **JLCPCB** as a two-layer board.

**Front Layer:** Contains all component placements and the 3.3V power traces. Traces 
route from the Arduino to each peripheral: the OLED connector, LED transistor bases, 
button inputs, potentiometer wiper, and buzzer driver. Components are grouped by 
function — input controls (RV1, SW1) sit in the lower section, while output and display 
components (OLED, LEDs, buzzer) occupy the upper portion of the board.

**Back Layer:** A full copper ground pour covers the back of the board. This provides a 
low-impedance return path for all signals, reduces EMI, and improves overall signal 
integrity. Traces that could not be completed on the front layer are routed through via 
holes and connected with short runs on the back.

**Layout Decisions:** The buzzer and C2 are placed on the right side to keep the 
switching circuitry away from the I2C signal lines running to the OLED. The Arduino 
sits at the bottom with the USB port accessible at the board edge for programming and 
power. The OLED connector (J1) is centrally positioned to keep I2C trace lengths short.

<img width="294" height="423" alt="Image" src="https://github.com/user-attachments/assets/6a6f89bf-1c50-467d-80c2-c18c8f27645a" />

**3D Views**

<img width="304" height="439" alt="Image" src="https://github.com/user-attachments/assets/290cabc9-8447-4586-b611-76094ca5f599" />
<img width="304" height="439" alt="Image" src="https://github.com/user-attachments/assets/fc7e3ecb-3d04-4fca-9fb0-20c1a068181b" />

---

## Power Management & Audio

The board runs entirely on **3.3V**, supplied through the Arduino Nano ESP32's onboard 
regulator via USB.

**C1 (0.1µF):** A decoupling capacitor placed close to the microcontroller's supply 
rail. It filters high-frequency switching noise that could cause erratic behavior or 
false reads on the input pins.

**C2 (10µF):** Placed across the buzzer supply line. When the buzzer activates, it 
draws a quick burst of current. Without this capacitor, that inrush could cause a brief 
voltage sag on the 3.3V rail and risk resetting the microcontroller. C2 acts as a local 
charge reservoir, absorbing that demand without disturbing the rest of the circuit.

**Buzzer (BZ1):** Driven via PWM through Q5. The frequency and duty cycle of the signal 
determine the pitch and volume of the tone. The alert fires when the countdown reaches 
zero, giving an audible signal even when the user is not watching the display.

---

## Usage

1. Power the board by connecting the Arduino Nano ESP32 via USB
2. The OLED will display the current date and time in idle mode
3. Turn **RV1** to dial in the desired number of minutes
4. Press **SW1** to fine-tune the seconds if needed
5. Press **SW2 (Start)** to begin the countdown — **D1 (Green)** will flash briefly as confirmation
6. Press **SW3 (Stop)** at any time to halt the timer — **D2 (Red)** will flash briefly
7. When the countdown reaches zero, **BZ1** fires an audible alert tone

---

## Design Philosophy

The goal was straightforward: build a focused physical timer that does not require a 
phone. Phone timers work, but they come with the friction of unlocking a screen and the 
temptation to check everything else on it. This board removes that entirely. There is 
one job, and every part on it serves that job.

The potentiometer was a deliberate choice over a simple button pair. Dialing in a time 
feels faster and more natural, especially for common use cases like study sessions, 
cooking intervals, or workout breaks. The physical controls give the device a tactile 
quality that a touchscreen does not replicate.

From an engineering standpoint, going from a hand-wired prototype to a manufactured PCB 
was the most important step in the project. Copper traces eliminate the reliability 
issues that come with jumper wires, the form factor is more compact, and the layout is 
repeatable. Designing in KiCad and fabricating through JLCPCB made this a practical 
exercise in the full hardware development cycle, from schematic capture all the way to 
a physical board.

---

## Images

**Schematic**

<img width="542" height="390" alt="Image" src="https://github.com/user-attachments/assets/751d345d-9680-4d3d-8154-dcac6ccd7f75" />

---

**PCB Layout**

<img width="294" height="423" alt="Image" src="https://github.com/user-attachments/assets/6a6f89bf-1c50-467d-80c2-c18c8f27645a" />

---

**3D Views**

<img width="304" height="439" alt="Image" src="https://github.com/user-attachments/assets/290cabc9-8447-4586-b611-76094ca5f599" />
<img width="304" height="439" alt="Image" src="https://github.com/user-attachments/assets/fc7e3ecb-3d04-4fca-9fb0-20c1a068181b" />

---

*Designed in KiCad 9.0.7 — Fabricated by JLCPCB — Built on Arduino Nano ESP32*
