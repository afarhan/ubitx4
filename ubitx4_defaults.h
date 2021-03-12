/**
 * This source file is under General Public License version 3.
 */

#ifndef	__UBITX4_EEPROM_DEFAULTS_H__
#define	__UBITX4_EEPROM_DEFAULTS_H__

// Delay when switching between TX and RX.
// This prevents spurious bursts when switching RX->TX.
#define TX_DELAY_OSC_OFF 15  // wait 15ms after turning off si5351 for it to be off off
#define TX_DELAY_ENABLE 30 // wait 30ms after enabling PTT to let things settle.

/**
 * The uBITX is an upconnversion transceiver. The first IF is at 45 MHz.
 * The first IF frequency is not exactly at 45 Mhz but about 5 khz lower,
 * this shift is due to the loading on the 45 Mhz crystal filter by the matching
 * L-network used on it's either sides.
 * The first oscillator works between 48 Mhz and 75 MHz. The signal is subtracted
 * from the first oscillator to arriive at 45 Mhz IF. Thus, it is inverted : LSB becomes USB
 * and USB becomes LSB.
 * The second IF of 12 Mhz has a ladder crystal filter. If a second oscillator is used at
 * 57 Mhz, the signal is subtracted FROM the oscillator, inverting a second time, and arrives
 * at the 12 Mhz ladder filter thus doouble inversion, keeps the sidebands as they originally were.
 * If the second oscillator is at 33 Mhz, the oscilaltor is subtracated from the signal,
 * thus keeping the signal's sidebands inverted. The USB will become LSB.
 * We use this technique to switch sidebands. This is to avoid placing the lsbCarrier close to
 * 12 MHz where its fifth harmonic beats with the arduino's 16 Mhz oscillator's fourth harmonic
 */

// the second oscillator should ideally be at 57 MHz, however, the crystal filter's center frequency
// is shifted down a little due to the loading from the impedance matching L-networks on either sides
#define SECOND_OSC_USB (56995000l)
#define SECOND_OSC_LSB (32995000l)

// the second oscillator should ideally be at 57 MHz, however, the crystal filter's center frequency
// is shifted down a little due to the loading from the impedance matching L-networks on either sides
#define SECOND_OSC_USB (56995000l)
#define SECOND_OSC_LSB (32995000l)

//these are the two default USB and LSB frequencies. The best frequencies depend upon your individual taste and filter shape
#define INIT_USB_FREQ   (11996500l)
// limits the tuning and working range of the ubitx between 3 MHz and 30 MHz
#define LOWEST_FREQ   (100000l)
#define HIGHEST_FREQ (30000000l)

#define DEFAULT_FIRSTIF 45000000L

#endif	/* __UBITX4_EEPROM_DEFAULTS_H__ */
