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
#include <Wire.h>
#include <EEPROM.h>

#include "ubitx4_eeprom_defs.h"
#include "ubitx4_pin_config.h"
#include "ubitx4_defaults.h"

#include "ubitx4_txfilt.h"

#include "ubitx_ui.h"

/**
    The main chip which generates upto three oscillators of various frequencies in the
    Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet
    from www.silabs.com although, strictly speaking it is not a requirment to understand this code.

    We no longer use the standard SI5351 library because of its huge overhead due to many unused
    features consuming a lot of program space. Instead of depending on an external library we now use
    Jerry Gaffke's, KE7ER, lightweight standalone mimimalist "si5351bx" routines (see further down the
    code). Here are some defines and declarations used by Jerry's routines:
*/

#include <LiquidCrystal.h>
LiquidCrystal lcd(LCD_PIN_RESET, LCD_PIN_ENABLE,
  LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);

/**
 * The Arduino, unlike C/C++ on a regular computer with gigabytes of RAM, has very little memory.
 * We have to be very careful with variables that are declared inside the functions as they are 
 * created in a memory region called the stack. The stack has just a few bytes of space on the Arduino
 * if you declare large strings inside functions, they can easily exceed the capacity of the stack
 * and mess up your programs. 
 * We circumvent this by declaring a few global buffers as  kitchen counters where we can 
 * slice and dice our strings. These strings are mostly used to control the display or handle
 * the input and output from the USB port. We must keep a count of the bytes used while reading
 * the serial port as we can easily run out of buffer space. This is done in the serial_in_count variable.
 */
char c[30], b[30];      
char printBuff[2][31];  //mirrors what is showing on the two lines of the display
int count = 0;          //to generally count ticks, loops, etc

//we directly generate the CW by programmin the Si5351 to the cw tx frequency, hence, both are different modes
//these are the parameter passed to startTx
#define TX_SSB 0
#define TX_CW 1

char ritOn = 0;
char vfoActive = VFO_A;
int8_t meter_reading = 0; // a -1 on meter makes it invisible
unsigned long vfoA=7150000L, vfoB=14200000L, sideTone=800, usbCarrier;
char isUsbVfoA=0, isUsbVfoB=1;
unsigned long frequency, ritRxFrequency, ritTxFrequency;  //frequency is the current frequency on the dial
unsigned long firstIF = DEFAULT_FIRSTIF;

//these are variables that control the keyer behaviour
int cwSpeed = 100; //this is actuall the dot period in milliseconds
extern int32_t calibration;
byte cwDelayTime = 60;
bool Iambic_Key = true;
#define IAMBICB 0x10 // 0 for Iambic A, 1 for Iambic B
unsigned char keyerControl = IAMBICB;


/**
 * Raduino needs to keep track of current state of the transceiver. These are a few variables that do it
 */
boolean txCAT = false;        //turned on if the transmitting due to a CAT command
char inTx = 0;                //it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
char splitOn = 0;             //working split, uses VFO B as the transmit frequency, (NOT IMPLEMENTED YET)
char keyDown = 0;             //in cw mode, denotes the carrier is being transmitted
char isUSB = 0;               //upper sideband was selected, this is reset to the default for the 
                              //frequency when it crosses the frequency border of 10 MHz
byte menuOn = 0;              //set to 1 when the menu is being displayed, if a menu item sets it to zero, the menu is exited
unsigned long cwTimeout = 0;  //milliseconds to go before the cw transmit line is released and the radio goes back to rx mode
unsigned long dbgCount = 0;   //not used now
unsigned char txFilter = 0;   //which of the four transmit filters are in use
boolean modeCalibrate = false;//this mode of menus shows extended menus to calibrate the oscillators and choose the proper
                              //beat frequency

/**
 * Below are the basic functions that control the uBitx. Understanding the functions before 
 * you start hacking around
 */

/**
 * Our own delay. During any delay, the raduino should still be processing a few times. 
 */

void active_delay(int delay_by){
  unsigned long timeStart = millis();

  while (millis() - timeStart <= delay_by) {
      //Background Work      
    checkCAT();
  }
}

/**
 * This is the most frequently called function that configures the 
 * radio to a particular frequeny, sideband and sets up the transmit filters
 * 
 * The transmit filter relays are powered up only during the tx so they dont
 * draw any current during rx. 
 * 
 * The carrier oscillator of the detector/modulator is permanently fixed at
 * uppper sideband. The sideband selection is done by placing the second oscillator
 * either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands 
 * through mixing of the second local oscillator.
 */
 
void setFrequency(unsigned long f){
  uint64_t osc_f, firstOscillator, secondOscillator;
 
  setTXFilters(f);

  if (isUSB){
    si5351bx_setfreq(2, firstIF  + f);
    si5351bx_setfreq(1, firstIF + usbCarrier);
  }
  else{
    si5351bx_setfreq(2, firstIF + f);
    si5351bx_setfreq(1, firstIF - usbCarrier);
  }
  
  frequency = f;
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */
 
void startTx(byte txMode){
  unsigned long tx_freq = 0;  

  // Turn off the main mixer oscillator and wait for RX->TX burst before
  // TX is flipped on.  Then we will re-enable the clocks.
  si5351bx_setfreq(2, 0);

  // Wait so things settle.
  delay(TX_DELAY_OSC_OFF);

  // Flip to TX.
  digitalWrite(TX_RX, 1);

  // Wait for relays, etc to settle.
  delay(TX_DELAY_ENABLE);
  inTx = 1;
  
  if (ritOn){
    //save the current as the rx frequency
    ritRxFrequency = frequency;
    setFrequency(ritTxFrequency);
  }
  else 
  {
    if (splitOn == 1) {
      if (vfoActive == VFO_B) {
        vfoActive = VFO_A;
        isUSB = isUsbVfoA;
        frequency = vfoA;
      }
      else if (vfoActive == VFO_A){
        vfoActive = VFO_B;
        frequency = vfoB;
        isUSB = isUsbVfoB;        
      }
    }
    setFrequency(frequency);
  }

  if (txMode == TX_CW){
    //turn off the second local oscillator and the bfo
    si5351bx_setfreq(0, 0);
    si5351bx_setfreq(1, 0);

    //shif the first oscillator to the tx frequency directly
    //the key up and key down will toggle the carrier unbalancing
    //the exact cw frequency is the tuned frequency + sidetone
    if (isUSB)
      si5351bx_setfreq(2, frequency + sideTone);
    else
      si5351bx_setfreq(2, frequency - sideTone); 
  }
  updateDisplay();
  clearMeterDisplay();
}

void stopTx(){
  inTx = 0;

  // Turn off the main vfo before disabling TX. setFrequency() will fix
  // it.
  si5351bx_setfreq(2, 0);           // disable the VFO oscillator.
  delay(TX_DELAY_OSC_OFF);

  digitalWrite(TX_RX, 0);           //turn off the tx
  delay(TX_DELAY_ENABLE);           // wait for relays to settle
  si5351bx_setfreq(0, usbCarrier);  //set back the cardrier oscillator anyway, cw tx switches it off

  if (ritOn)
    setFrequency(ritRxFrequency);
  else{
    if (splitOn == 1) {
      //vfo Change
      if (vfoActive == VFO_B){
        vfoActive = VFO_A;
        frequency = vfoA;
        isUSB = isUsbVfoA;        
      }
      else if (vfoActive == VFO_A){
        vfoActive = VFO_B;
        frequency = vfoB;
        isUSB = isUsbVfoB;        
      }
    }
    setFrequency(frequency);
  }
  updateDisplay();
}

/**
 * ritEnable is called with a frequency parameter that determines
 * what the tx frequency will be
 */
void ritEnable(unsigned long f){
  ritOn = 1;
  //save the non-rit frequency back into the VFO memory
  //as RIT is a temporary shift, this is not saved to EEPROM
  ritTxFrequency = f;
}

// this is called by the RIT menu routine
void ritDisable(){
  if (ritOn){
    ritOn = 0;
    setFrequency(ritTxFrequency);
    updateDisplay();
  }
}

/**
 * Basic User Interface Routines. These check the front panel for any activity
 */

/**
 * The PTT is checked only if we are not already in a cw transmit session
 * If the PTT is pressed, we shift to the ritbase if the rit was on
 * flip the T/R line to T and update the display to denote transmission
 */

void checkPTT(){	
  //we don't check for ptt when transmitting cw
  if (cwTimeout > 0)
    return;
    
  if (digitalRead(PTT) == 0 && inTx == 0){
    startTx(TX_SSB);
    active_delay(50); //debounce the PTT
  }
	
  if (digitalRead(PTT) == 1 && inTx == 1)
    stopTx();
}

void checkButton(){
  int i, t1, t2, knob, new_knob;

  //only if the button is pressed
  if (!btnDown())
    return;
  active_delay(50);
  if (!btnDown()) //debounce
    return;
 
  doMenu();
  //wait for the button to go up again
  while(btnDown())
    active_delay(10);
  active_delay(50);//debounce
}


/**
 * The tuning jumps by 50 Hz on each step when you tune slowly
 * As you spin the encoder faster, the jump size also increases 
 * This way, you can quickly move to another band by just spinning the 
 * tuning knob
 */


void doTuning(){
  int s;
  unsigned long prev_freq;

  s = enc_read();
  if (s != 0){
    prev_freq = frequency;

    if (s > 4)
      frequency += 10000l;
    else if (s > 2)
      frequency += 500;
    else if (s > 0)
      frequency +=  50l;
    else if (s > -2)
      frequency -= 50l;
    else if (s > -4)
      frequency -= 500l;
    else
      frequency -= 10000l;

    if (prev_freq < 10000000l && frequency > 10000000l)
      isUSB = true;
      
    if (prev_freq > 10000000l && frequency < 10000000l)
      isUSB = false;

    setFrequency(frequency);
    updateDisplay();
  }
}

/**
 * RIT only steps back and forth by 100 hz at a time
 */
void doRIT(){
  unsigned long newFreq;
 
  int knob = enc_read();
  unsigned long old_freq = frequency;

  if (knob < 0)
    frequency -= 100l;
  else if (knob > 0)
    frequency += 100;
 
  if (old_freq != frequency){
    setFrequency(frequency);
    updateDisplay();
  }
}

/**
 * The settings are read from EEPROM. The first time around, the values may not be 
 * present or out of range, in this case, some intelligent defaults are copied into the 
 * variables.
 */
void initSettings(){
  byte x;
  //read the settings from the eeprom and restore them
  //if the readings are off, then set defaults
  EEPROM.get(MASTER_CAL, calibration);
  EEPROM.get(USB_CAL, usbCarrier);
  EEPROM.get(VFO_A, vfoA);
  EEPROM.get(VFO_B, vfoB);
  EEPROM.get(CW_SIDETONE, sideTone);
  EEPROM.get(CW_SPEED, cwSpeed);
  EEPROM.get(CAL_FIRST_IF_FREQ, firstIF);
  
  if (usbCarrier > 12000000l || usbCarrier < 11990000l)
    usbCarrier = INIT_USB_FREQ;
  if (vfoA > 35000000l || 3500000l > vfoA)
     vfoA = 7150000l;
  if (vfoB > 35000000l || 3500000l > vfoB)
     vfoB = 14150000l;  
  if (sideTone < 100 || 2000 < sideTone) 
    sideTone = 800;
  if (cwSpeed < 10 || 1000 < cwSpeed) 
    cwSpeed = 100;
  if (firstIF < 44500000 || firstIF > 45500000)
    firstIF = DEFAULT_FIRSTIF;

  /*
   * The VFO modes are read in as either 2 (USB) or 3(LSB), 0, the default
   * is taken as 'uninitialized
   */

  EEPROM.get(VFO_A_MODE, x);
 
  switch(x){
    case VFO_MODE_USB:
      isUsbVfoA = 1;
      break;
    case VFO_MODE_LSB:
      isUsbVfoA = 0;
      break;
    default:
      if (vfoA > 10000000l)
        isUsbVfoA = 1;
      else 
        isUsbVfoA = 0;      
  }

  EEPROM.get(VFO_B_MODE, x);
  switch(x){
    case VFO_MODE_USB:
      isUsbVfoB = 1;
      break;
    case VFO_MODE_LSB:
      isUsbVfoB = 0;
      break;
    default:
      if (vfoA > 10000000l)
        isUsbVfoB = 1;
      else 
        isUsbVfoB = 0;      
  }

  //set the current mode
  isUSB = isUsbVfoA;

  /*
   * The keyer type splits into two variables
   */
   EEPROM.get(CW_KEY_TYPE, x);

   if (x == 0)
    Iambic_Key = false;
  else if (x == 1){
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (x == 2){
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
  
}

void initPorts(){

  analogReference(DEFAULT);

  //??
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(FBUTTON, INPUT_PULLUP);
  
  //configure the function button to use the external pull-up
//  pinMode(FBUTTON, INPUT);
//  digitalWrite(FBUTTON, HIGH);

  pinMode(PTT, INPUT_PULLUP);
  pinMode(ANALOG_KEYER, INPUT_PULLUP);

  pinMode(CW_TONE, OUTPUT);  
  digitalWrite(CW_TONE, 0);
  
  pinMode(TX_RX,OUTPUT);
  digitalWrite(TX_RX, 0);

  pinMode(TX_LPF_A, OUTPUT);
  pinMode(TX_LPF_B, OUTPUT);
  pinMode(TX_LPF_C, OUTPUT);
  digitalWrite(TX_LPF_A, 0);
  digitalWrite(TX_LPF_B, 0);
  digitalWrite(TX_LPF_C, 0);

  pinMode(CW_KEY, OUTPUT);
  digitalWrite(CW_KEY, 0);
}

void setup()
{
  Serial.begin(38400);
  Serial.flush();
  lcd.begin(16, 2);
  lcd.clear();
  initMeter();

  //we print this line so this shows up even if the raduino 
  //crashes later in the code
  printLine2("uBITX v4.3b");
  //active_delay(500);

  initSettings();
  initPorts();     
  initOscillators();

  si5351bx_set_drive(0, 1); // 4ma
  si5351bx_set_drive(1, 1); // 4ma
  si5351bx_set_drive(2, 1); // 4ma

  frequency = vfoA;
  setFrequency(vfoA);
  updateDisplay();

  if (btnDown())
    factory_alignment();
}


/**
 * The loop checks for keydown, ptt, function button and tuning.
 */

byte flasher = 0;
void loop(){

  cwKeyer();
  if (!txCAT)
    checkPTT();
  checkButton();

  //tune / display s-meter only when not tranmsitting
  if (!inTx){
    if (ritOn)
      doRIT();
    else 
      doTuning();
    // TODO: only update this every few milliseconds? Not constantly?
    updateMeterDisplay();
  }

  //we check CAT after the encoder as it might put the radio into TX
  checkCAT();
}
