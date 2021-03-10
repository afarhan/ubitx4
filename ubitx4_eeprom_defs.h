/**
 * This source file is under General Public License version 3.
 *
 * This verision uses a built-in Si5351 library
 * Most source code are meant to be understood by the compilers and the computers. 
 * Code that has to be hackable needs to be well understood and properly documented. 
 * Donald Knuth coined the term Literate Programming to indicate code that is written be 
 * easily read and understood.
 *
 * The Raduino is a small board that includes the Arduin Nano, a 16x2 LCD display and
 * an Si5351a frequency synthesizer. This board is manufactured by Paradigm Ecomm Pvt Ltd
 *
 * To learn more about Arduino you may visit www.arduino.cc. 
 *
 * The Arduino works by starts executing the code in a function called setup() and then it 
 * repeatedly keeps calling loop() forever. All the initialization code is kept in setup()
 * and code to continuously sense the tuning knob, the function button, transmit/receive,
 * etc is all in the loop() function. If you wish to study the code top down, then scroll
 * to the bottom of this file and read your way up.
 *
 * Below are the libraries to be included for building the Raduino 
 * The EEPROM library is used to store settings like the frequency memory, caliberation data, 
 * callsign etc .
 *
 *  The main chip which generates upto three oscillators of various frequencies in the
 *  Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet 
 *  from www.silabs.com although, strictly speaking it is not a requirment to understand this code. 
 *  Instead, you can look up the Si5351 library written by xxx, yyy. You can download and 
 *  install it from www.url.com to complile this file.
 *  The Wire.h library is used to talk to the Si5351 and we also declare an instance of 
 *  Si5351 object to control the clocks.
 */

#ifndef	__UBITX4_EEPROM_DEFS_H__
#define	__UBITX4_EEPROM_DEFS_H__

/**
 * These are the indices where these user changable settinngs are stored  in the EEPROM
 */
#define MASTER_CAL 0
#define LSB_CAL 4
#define USB_CAL 8
#define SIDE_TONE 12
//these are ids of the vfos as well as their offset into the eeprom storage, don't change these 'magic' values
#define VFO_A 16
#define VFO_B 20
#define CW_SIDETONE 24
#define CW_SPEED 28

// Adrian's custom values
#define CAL_FIRST_IF_FREQ 252 // 252-255 - first IF freq calibrated value

//These are defines for the new features back-ported from KD8CEC's software
//these start from beyond 256 as Ian, KD8CEC has kept the first 256 bytes free for the base version
#define VFO_A_MODE  256 // 2: LSB, 3: USB
#define VFO_B_MODE  257

//values that are stored for the VFO modes
#define VFO_MODE_LSB 2
#define VFO_MODE_USB 3

// handkey, iambic a, iambic b : 0,1,2f
#define CW_KEY_TYPE 358

#endif	/* __UBITX4_EEPROM_DEFS_H__ */
