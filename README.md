# Digital-Computer-Structure---Final-Project

Final project for the Digital Computer Structure course. This project uses the MSP430G2553 to design and implement a stepper motor control system using a joystick, a PC painter mode, and a script mode for automated control, all managed through an intuitive graphical user interface (GUI) on the PC side.

## Features

### 1. Manual Control Using Joystick
In this mode, the user controls the stepper motor using a joystick. The joystick being pointed in a certain angle and the motor points to the same angle: the joystick location is sampled by the ADC10, and the values are passed into the `atan2` function to calculate the joystick pointing angle. The system translates the joystick’s tilt into motor movement to provide smooth and precise control.

### 2. Joystick-Based Painter
The joystick is also used in the painter mode, where it controls a pointer on a canvas on the PC. The GUI allows users to paint on the canvas by moving the joystick and selecting between three modes: pen, eraser, and neutral. This mode uses the ADC to sample the joystick angle to determine cursor movement direction, and UART communication is used to relay joystick position data to the PC in real-time, providing a seamless painting experience.

### 3. Stepper Motor Calibration
This mode is responsible for calibrating the stepper motor. The motor is rotated in controlled steps, and the calibration is used to determine the exact step size for accurate positioning. This is important for precise performance in the motor control states. The calibration data is stored in the flash memory (non-volatile memory), ensuring that motor movements remain precise even after the MSP430 is powered off.

### 4. Script Mode
In script mode, users can browse predefined scripts from the PC, which control the motor and the LCD screen of the MSP430 based on a sequence of commands. The script is decoded on the PC side and then sent via UART to the MSP430, where it is burned to the flash memory. The MSP430 sends an ACK to confirm the data is written to flash. After that, the user can execute the decoded script. The MSP430 sends another ACK when the execution is complete, allowing for automated movements and complex sequences.

## Peripheral Usage

### 1. ADC10 (Analog-to-Digital Converter)
The ADC10 samples the analog signals from the joystick, converting the joystick’s movement (voltages) into digital values that can be processed by the microcontroller. These values are then used in both manual control mode and the painter mode to calculate the corresponding motor rotation or pointer movement.

### 2. UART (Universal Asynchronous Receiver-Transmitter)
UART is used for serial communication between the MSP430 and the PC. It transmits data such as joystick position, motor angles, and script commands between the two devices. The GUI on the PC interacts with the microcontroller by sending and receiving data via UART, enabling real-time control and feedback.

### 3. Timers
Timers play a crucial role in generating precise delays required for controlling the motor's speed and movement. They also help in triggering periodic ADC10 conversions to read the joystick inputs. In script mode, timers ensure that the motor movements follow the timing specified in the scripts.



## Example Script and Decoded Script
inc_lcd 3 --------------> 0103
set_delay 30 -----------> 041E
dec_lcd 3 --------------> 0203
rra_lcd 4 --------------> 0334
clear_lcd --------------> 05
stepper_deg 35 ---------> 0623
inc_lcd 5 --------------> 0105
stepper_scan 20,60 -----> 07143C
sleep ------------------> 08


Explenation:  
inc_lcd 3: Increases the LCD value by 3 with delay d (0103). inc_lcd opcode = 0x01, and has 1 HEX operand.
set_delay 30: Sets a delay of 30 units (30*10ms) (041E). set_delay  opcode = 0x04 and has 1 HEX operand.
dec_lcd 3: Decreases the LCD value by 3 with delay d (0203). dec_leds  opcode = 0x02, has 1 HEX operand.
rra_lcd A: Rotates the LCD display right with char '4' (0334). rrc_lcd  opcode = 0x03, and has 1 HEX operand.
clear_lcd: Clears the LCD screen (05). clear_lcd  opcode = 0x05 and has no operand.
stepper_deg 35: Moves the stepper motor to 35 degrees (0623). stepper_deg  opcode = 0x06 and has 1 HEX operand .
stepper_scan 20, 60: Scans the stepper motor between 20 and 60 degrees (07143C). stepper_scan  opcode = 0x07 and has 2 HEX operands.
sleep: Puts the system to sleep (08). sleep  opcode = 0x08 and has no operands.



## Project Structure

### `main.c`
Contains the main Finite State Machine (FSM) states that control the overall flow of the application. It handles transitions between different modes (Manual Control, Joystick Painter, Calibration, Script Mode) based on user input from the PC or joystick.

### `api.c`
Implements high-level application functions that interface with the main FSM, providing the core functionality of the application. These functions include motor control logic, communication with the PC, and handling of script execution.

### `halGPIO.c`
Includes low-level hardware abstraction layer (HAL) functions for GPIO operations, supporting the control and operation of application-level functions, such as the joystick, motor control, and the flash memory operations.

### `bsp.c`
Handles the initialization of hardware modules specific to the MSP430 board, such as setting up the ADC10, UART, Timers, and other peripherals needed for the application to function correctly.

### `Headers`
All header files include declarations of external functions and variables, making them accessible across different files in the project.

### `PC_Side.py`
A Python script designed to establish communication between the MSP430 microcontroller and a PC. The script allows the PC to control the microcontroller's functions via a graphical user interface (GUI). The GUI provides control buttons for manual motor control, the joystick-based painter, and the ability to upload and execute scripts in script mode.
