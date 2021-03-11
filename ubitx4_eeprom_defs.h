/**
 * This source file is under General Public License version 3.
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
