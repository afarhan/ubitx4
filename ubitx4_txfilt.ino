/**
 * This source file is under General Public License version 3.
 */

#include "ubitx4_eeprom_defs.h"
#include "ubitx4_pin_config.h"

/**
 * Select the properly tx harmonic filters
 *
 * This is valid for the ubitx v4 board; different boards have different TX filter
 * configurations and thus will need a different filter mapping routine here!
 * 
 * The four harmonic filters use only three relays
 * the four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz
 * Briefly, it works like this, 
 * - When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
 * - When KT1 is ON, it routes the PA output to KT2. Which is why you will see that
 *   the KT1 is on for the three other cases.
 * - When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output
 *   to 18 MHz LPF (That also works for 14 Mhz) 
 * - When KT1 is On, KT2 is On, it routes the PA output to KT3
 * - KT3, when switched on selects the 7-10 Mhz filter
 * - KT3 when switched off selects the 3.5-5 Mhz filter
 * See the circuit to understand this
 */

void
setTXFilters(unsigned long freq)
{
  if (freq > 21000000L){  // the default filter is with 35 MHz cut-off
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq >= 14000000L){
    /*
     * throw the KT1 relay on, the 30 MHz LPF is bypassed and
     * the 14-18 MHz LPF is allowedd to go through
     */
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
  }
  else if (freq > 7000000L){
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
  }
  else {
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 1);
  }
}
