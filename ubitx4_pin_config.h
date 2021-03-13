/**
 * This source file is under General Public License version 3.
 */

#ifndef	__UBITX4_PIN_CONFIG_H__
#define	__UBITX4_PIN_CONFIG_H__

/**
 * We need to carefully pick assignment of pin for various purposes.
 * There are two sets of completely programmable pins on the Raduino.
 * First, on the top of the board, in line with the LCD connector is an 8-pin connector
 * that is largely meant for analog inputs and front-panel control. It has a regulated 5v output,
 * ground and six pins. Each of these six pins can be individually programmed 
 * either as an analog input, a digital input or a digital output. 
 * The pins are assigned as follows (left to right, display facing you): 
 *      Pin 1 (Violet), A7, S-METER / SPARE
 *      Pin 2 (Blue),   A6, KEYER (DATA)
 *      Pin 3 (Green), +5v 
 *      Pin 4 (Yellow), Gnd
 *      Pin 5 (Orange), A3, PTT
 *      Pin 6 (Red),    A2, F BUTTON
 *      Pin 7 (Brown),  A1, ENC B
 *      Pin 8 (Black),  A0, ENC A
 *Note: A5, A4 are wired to the Si5351 as I2C interface 
 *       *
 * - A0 and A1 are used for the rotary encoder
 * - A2 is used for the menu push button
 * - A3 is PTT to the microphone
 * - A6 is hooked up to the CW key jack via resistors, to implement
 *   either a straight key or paddle support
 * - A7 is spare, but is currently used to drive an S-meter menu
 */

#define ENC_A (A0)
#define ENC_B (A1)
#define FBUTTON (A2)
#define PTT   (A3)
#define ANALOG_KEYER (A6)
#define ANALOG_SPARE (A7)

/*
 * The 16x2 LCD panel is connected to digital pins as:
 * RESET : 8
 * ENABLE : 9
 * D4 : 10
 * D5 : 11
 * D6 : 12
 * D7 : 13
 */
#define LCD_PIN_RESET 8
#define LCD_PIN_ENABLE 9
#define LCD_PIN_D4 10
#define LCD_PIN_D5 11
#define LCD_PIN_D6 12
#define LCD_PIN_D7 13
/** 
 *  The second set of 16 pins on the Raduino's bottom connector are have the
 * three clock outputs and the digital lines to control the rig.
 *  This assignment is as follows :
 *    Pin   1   2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 *         GND +5V CLK0  GND  GND  CLK1 GND  GND  CLK2  GND  D2   D3   D4   D5   D6   D7
 *  These too are flexible with what you may do with them, for the Raduino, we use them to :
 *  - TX_RX line : Switches between Transmit and Receive after sensing the PTT or the morse keyer
 *  - CW_KEY line : turns on the carrier for CW
 */

#define TX_RX (7)
#define CW_TONE (6)
#define TX_LPF_A (5)
#define TX_LPF_B (4)
#define TX_LPF_C (3)
#define CW_KEY (2)

#endif	/* __UBITX4_PIN_CONFIG_H__ */
