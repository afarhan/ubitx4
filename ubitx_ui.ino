/**
 * The user interface of the ubitx consists of the encoder, the push-button on top of it
 * and the 16x2 LCD display.
 * The upper line of the display is constantly used to display frequency and status
 * of the radio. Occasionally, it is used to provide a two-line information that is 
 * quickly cleared up.
 */

#include "ubitx4_eeprom_defs.h"
#include "ubitx4_pin_config.h"

#include "ubitx_ui.h"

//returns true if the button is pressed
int btnDown(void)
{
  if (digitalRead(FBUTTON) == HIGH)
    return 0;
  else
    return 1;
}

/**
 * Meter (not used in this build for anything)
 * the meter is drawn using special characters. Each character is composed of 5 x 8 matrix.
 * The  s_meter array holds the definition of the these characters. 
 * each line of the array is is one character such that 5 bits of every byte 
 * makes up one line of pixels of the that character (only 5 bits are used)
 * The current reading of the meter is assembled in the string called meter
 */


char meter[2];

const PROGMEM uint8_t s_meter_bitmap[64] = {
  B00000, B00000, B00000, B00000, B00000, B00000, B00000, B11111, //shortest bar
  B00000, B00000, B00000, B00000, B00000, B00000, B11111, B11111,
  B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111,
  B00000, B00000, B00000, B00000, B11111, B11111, B11111, B11111,
  B00000, B00000, B00000, B11111, B11111, B11111, B11111, B11111,
  B00000, B00000, B11111, B11111, B11111, B11111, B11111, B11111,
  B00000, B11111, B11111, B11111, B11111, B11111, B11111, B11111,
  B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111, // tallest bar
};

// Initialise the custom character out of program memory
static void
initLcdChar(char lcd_char, char s_meter_bitmap_offset)
{
  uint8_t tmp_bytes[8];
  char i;

  for (i = 0; i < 8; i++) {
    tmp_bytes[i] = pgm_read_byte(&s_meter_bitmap[s_meter_bitmap_offset + i]);
  }
  lcd.createChar(lcd_char, tmp_bytes);
}

// initializes the custom characters
void initMeter(void)
{
  lcd.setCursor(0, 0); // Ensure we've reset to display RAM
  initLcdChar(0, 0); // First call - points to chargen RAM
  initLcdChar(1, 8);
  initLcdChar(2, 16);
  initLcdChar(3, 24);
  initLcdChar(4, 32);
  initLcdChar(5, 40);
  initLcdChar(6, 48);
  initLcdChar(7, 56);
  lcd.setCursor(0, 0); // Finish up - point to display RAM again
}


/**
 * The meter is drawn with special characters from 0..255
 */
void drawMeter(uint8_t needle)
{
  meter[0] = needle / 32;
  meter[1] = needle / 32;
}

// The generic routine to display one line on the LCD 
void printLine(char linenmbr, const char *c) {
  if (strcmp(c, printBuff[linenmbr])) {     // only refresh the display when there was a change
    lcd.setCursor(0, linenmbr);             // place the cursor at the beginning of the selected line
    lcd.print(c);
    strcpy(printBuff[linenmbr], c);

    for (byte i = strlen(c); i < 16; i++) { // add white spaces until the end of the 16 characters line is reached
      lcd.print(' ');
    }
  }
}


//  short cut to print to the first line
void printLine1(const char *c){
  printLine(1,c);
}
//  short cut to print to the first line
void printLine2(const char *c){
  printLine(0,c);
}

// this builds up the top line of the display with frequency and mode
void updateDisplay(void)
{
  // tks Jack Purdum W8TEE
  // replaced fsprint commmands by str commands for code size reduction

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(frequency, b, DEC);

  if (inTx){
    if (cwTimeout > 0)
      strcpy(c, "   CW:");
    else
      strcpy(c, "   TX:");
  }
  else {
    if (ritOn)
      strcpy(c, "RIT ");
    else {
      if (isUSB)
        strcpy(c, "USB ");
      else
        strcpy(c, "LSB ");
    }
    if (vfoActive == VFO_A) // VFO A is active
      strcat(c, "A:");
    else
      strcat(c, "B:");
  }



  //one mhz digit if less than 10 M, two digits if more
  if (frequency < 10000000l){
    c[6] = ' ';
    c[7]  = b[0];
    strcat(c, ".");
    strncat(c, &b[1], 3);    
    strcat(c, ".");
    strncat(c, &b[4], 3);
  }
  else {
    strncat(c, b, 2);
    strcat(c, ".");
    strncat(c, &b[2], 3);
    strcat(c, ".");
    strncat(c, &b[5], 3);    
  }

  if (inTx)
    strcat(c, " TX");
  printLine(1, c);
}

/*
 * Update the meter display!
 */
void
updateMeterDisplay(void)
{
  int meter_reading = 0;

  // Second line, meter on A7 (ANALOG_SPARE)
  meter_reading = analogRead(ANALOG_SPARE);

  // TODO - there's no mapping table here yet! Just linear it!
  if (meter_reading > 255) {
    // Ensure we definitely don't overflow an int here
    meter_reading = 255;
  }
  drawMeter(meter_reading); // this takes uint8_t
  lcd.setCursor(14, 0);
  lcd.write(meter[0]);
  lcd.setCursor(15, 0);
  lcd.write(meter[1]);
}

/*
 * Clear the meter - used during transmit
 */
void
clearMeterDisplay(void)
{
  lcd.setCursor(14, 0);
  lcd.write(' ');
  lcd.setCursor(15, 0);
  lcd.write(' ');
}

int enc_prev_state = 3;

/**
 * The A7 And A6 are purely analog lines on the Arduino Nano
 * These need to be pulled up externally using two 10 K resistors
 * 
 * There are excellent pages on the Internet about how these encoders work
 * and how they should be used. We have elected to use the simplest way
 * to use these encoders without the complexity of interrupts etc to 
 * keep it understandable.
 * 
 * The enc_state returns a two-bit number such that each bit reflects the current
 * value of each of the two phases of the encoder
 * 
 * The enc_read returns the number of net pulses counted over 50 msecs. 
 * If the puluses are -ve, they were anti-clockwise, if they are +ve, the
 * were in the clockwise directions. Higher the pulses, greater the speed
 * at which the enccoder was spun
 */

byte enc_state (void) {
    return (analogRead(ENC_A) > 500 ? 1 : 0) + (analogRead(ENC_B) > 500 ? 2: 0);
}

int enc_read(void) {
  int result = 0; 
  byte newState;
  int enc_speed = 0;
  
  long stop_by = millis() + 50;
  
  while (millis() < stop_by) { // check if the previous state was stable
    newState = enc_state(); // Get current state  
    
    if (newState != enc_prev_state)
      delay (1);
    
    if (enc_state() != newState || newState == enc_prev_state)
      continue; 
    //these transitions point to the encoder being rotated anti-clockwise
    if ((enc_prev_state == 0 && newState == 2) || 
      (enc_prev_state == 2 && newState == 3) || 
      (enc_prev_state == 3 && newState == 1) || 
      (enc_prev_state == 1 && newState == 0)){
        result--;
      }
    //these transitions point o the enccoder being rotated clockwise
    if ((enc_prev_state == 0 && newState == 1) || 
      (enc_prev_state == 1 && newState == 3) || 
      (enc_prev_state == 3 && newState == 2) || 
      (enc_prev_state == 2 && newState == 0)){
        result++;
      }
    enc_prev_state = newState; // Record state for next pulse interpretation
    enc_speed++;
    active_delay(1);
  }
  return(result);
}
