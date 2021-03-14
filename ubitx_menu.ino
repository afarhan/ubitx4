/** Menus
 *  The Radio menus are accessed by tapping on the function button. 
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press. 
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter. 
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */


/** A generic control to read variable values
*/
int getValueByKnob(int minimum, int maximum, int step_size,  int initial, const char* prefix, const char *postfix)
{
    int knob = 0;
    int knob_value;

    while (btnDown())
      active_delay(100);

    active_delay(200);
    knob_value = initial;
     
    strcpy(b, prefix);
    itoa(knob_value, c, 10);
    strcat(b, c);
    strcat(b, postfix);
    printLine2(b);
    active_delay(300);

    while(!btnDown() && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (knob_value > minimum && knob < 0)
          knob_value -= step_size;
        if (knob_value < maximum && knob > 0)
          knob_value += step_size;

        printLine2(prefix);
        itoa(knob_value, c, 10);
        strcpy(b, c);
        strcat(b, postfix);
        printLine1(b);
      }
      checkCAT();
    }

   return knob_value;
}

//# Menu: 1

void menuBand(int btn){
  int knob = 0;
  int band;
  unsigned long offset;

 // band = frequency/1000000l;
 // offset = frequency % 1000000l;
    
  if (!btn){
   printLineF2(F("Band Select    \x7E"));
   return;
  }

  printLineF2(F("Band Select:"));
  //wait for the button menu select button to be lifted)
  while (btnDown())
    active_delay(50);
  active_delay(50);    
  ritDisable();

  while(!btnDown()){

    knob = enc_read();
    if (knob != 0){
      /*
      if (band > 3 && knob < 0)
        band--;
      if (band < 30 && knob > 0)
        band++; 
      if (band > 10)
        isUSB = true;
      else
        isUSB = false;
      setFrequency(((unsigned long)band * 1000000l) + offset); */
      if (knob < 0 && frequency > 3000000l)
        setFrequency(frequency - 200000l);
      if (knob > 0 && frequency < 30000000l)
        setFrequency(frequency + 200000l);
      if (frequency > 10000000l)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
    }
    checkCAT();
    active_delay(20);
  }

  while(btnDown())
    active_delay(50);
  active_delay(50);
  
  printLineF2(F(""));
  updateDisplay();
  menuOn = 0;
}

// Menu #2
void menuRitToggle(int btn){
  if (!btn){
    if (ritOn == 1)
      printLineF2(F("RIT On \x7E Off"));
    else
      printLineF2(F("RIT Off \x7E On"));
  }
  else {
    if (ritOn == 0){
      //enable RIT so the current frequency is used at transmit
      ritEnable(frequency);
      printLineF2(F("RIT is On"));
 
    }
    else{
      ritDisable();
      printLineF2(F("RIT is Off"));
    }
    menuOn = 0;
    active_delay(500);
    printLineF2(F(""));
    updateDisplay();
  }
}


//Menu #3
void menuVfoToggle(int btn){
  
  if (!btn){
    if (vfoActive == VFO_A)
      printLineF2(F("VFO A \x7E B"));
    else
      printLineF2(F("VFO B \x7E A"));
  }
  else {
    if (vfoActive == VFO_B){
      vfoB = frequency;
      isUsbVfoB = isUSB;
      EEPROM.put(VFO_B, frequency);
      if (isUsbVfoB)
        EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);

      vfoActive = VFO_A;
//      printLineF2(F("Selected VFO A  "));
      frequency = vfoA;
      isUSB = isUsbVfoA;
    }
    else {
      vfoA = frequency;
      isUsbVfoA = isUSB;
      EEPROM.put(VFO_A, frequency);
      if (isUsbVfoA)
        EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);

      vfoActive = VFO_B;
//      printLineF2(F("Selected VFO B  "));
      frequency = vfoB;
      isUSB = isUsbVfoB;
    }
      
    ritDisable();
    setFrequency(frequency);
    updateDisplay();
    printLineF2(F(""));
      //exit the menu
    menuOn = 0;
  }
}

// Menu #4
void menuSidebandToggle(int btn){
  if (!btn){
    if (isUSB == true)
      printLineF2(F("USB \x7E LSB"));
    else
      printLineF2(F("LSB \x7E USB"));
  }
  else {
    if (isUSB == true){
      isUSB = false;
      printLineF2(F("LSB Selected"));
      active_delay(500);
      printLineF2(F(""));
    }
    else {
      isUSB = true;
      printLineF2(F("USB Selected"));
      active_delay(500);
      printLineF2(F(""));
    }
    //Added by KD8CEC
    if (vfoActive == VFO_B){
      isUsbVfoB = isUSB;
    }
    else {
      isUsbVfoA = isUSB;
    }
    updateDisplay();

    // Update the programmed frequency immediately so the band flips.
    setFrequency(frequency);

    menuOn = 0;
  }
}

//Split communication using VFOA and VFOB by KD8CEC
//Menu #5
void menuSplitToggle(int btn){
  if (!btn){
    if (splitOn == 0)
      printLineF2(F("Split Off \x7E On"));
    else
      printLineF2(F("Split On \x7E Off"));
  }
  else {
    if (splitOn == 1){
      splitOn = 0;
      printLineF2(F("Split ON"));
    }
    else {
      splitOn = 1;
      if (ritOn == 1)
        ritOn = 0;
      printLineF2(F("Split Off"));
    }
    active_delay(500);
    printLineF2(F(""));
    updateDisplay();
    menuOn = 0;
  }
}

void menuCWSpeed(int btn){
    int knob = 0;
    int wpm;

    wpm = 1200/cwSpeed;
     
    if (!btn){
     strcpy(b, "CW: ");
     itoa(wpm,c, 10);
     strcat(b, c);
     strcat(b, " WPM     \x7E");
     printLine2(b);
     return;
    }

/*
    printLine1("Press FN to Set");
    strcpy(b, "5:CW>");
    itoa(wpm,c, 10);
    strcat(b, c);
    strcat(b, " WPM");
    printLine2(b);
    active_delay(300);

    while(!btnDown() && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (wpm > 3 && knob < 0)
          wpm--;
        if (wpm < 50 && knob > 0)
          wpm++;

        strcpy(b, "5:CW>");
        itoa(wpm,c, 10);
        strcat(b, c);
        strcat(b, " WPM");
        printLine2(b);
      }
      //abort if this button is down
      if (btnDown())
        //re-enable the clock1 and clock 2
        break;
      checkCAT();
    }
  */
    wpm = getValueByKnob(1, 100, 1,  wpm, "CW: ", " WPM>");
  
    printLineF2(F("CW Speed set!"));
    cwSpeed = 1200/wpm;
    EEPROM.put(CW_SPEED, cwSpeed);
    active_delay(500);
    
    printLineF2(F(""));
    updateDisplay();
    menuOn = 0;
}

void menuExit(int btn){

  if (!btn){
      printLineF2(F("Exit Menu      \x7E"));
  }
  else{
      printLineF2(F("Exiting..."));
      active_delay(500);
      printLineF2(F(""));
      updateDisplay();
      menuOn = 0;
  }
}

/**
 * The calibration routines are not normally shown in the menu as they are rarely used
 * They can be enabled by choosing this menu option
 */
int menuSetup(int btn){
  if (!btn){
    if (!modeCalibrate)
      printLineF2(F("Settings       \x7E"));
    else
      printLineF2(F("Settings \x7E Off"));
  }else {
    if (!modeCalibrate){
      modeCalibrate = true;
      printLineF2(F("Settings On"));
    }
    else {
      modeCalibrate = false;
      printLineF2(F("Settings Off"));
    }

   while(btnDown())
    active_delay(100);
   active_delay(500);
   printLineF2(F(""));
   return 10;
  }
  return 0;
}

//this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;

int calibrateClock(){
  int knob = 0;
  int32_t prev_calibration;


  //keep clear of any previous button press
  while (btnDown())
    active_delay(100);
  active_delay(100);
   
  digitalWrite(TX_LPF_A, 0);
  digitalWrite(TX_LPF_B, 0);
  digitalWrite(TX_LPF_C, 0);

  prev_calibration = calibration;
  calibration = 0;

  isUSB = true;

  //turn off the second local oscillator and the bfo
  si5351_set_calibration(calibration);
  startTx(TX_CW);
  si5351bx_setfreq(2, 10000000l); 
  
  strcpy(b, "#1 10 MHz cal:");
  ltoa(calibration/8750, c, 10);
  strcat(b, c);
  printLine2(b);     

  while (!btnDown())
  {

    if (digitalRead(PTT) == LOW && !keyDown)
      cwKeydown();
    if (digitalRead(PTT)  == HIGH && keyDown)
      cwKeyUp();
      
    knob = enc_read();

    if (knob > 0)
      calibration += 875;
    else if (knob < 0)
      calibration -= 875;
    else 
      continue; //don't update the frequency or the display
      
    si5351_set_calibration(calibration);
    si5351bx_setfreq(2, 10000000l);
    strcpy(b, "#1 10 MHz cal:");
    ltoa(calibration/8750, c, 10);
    strcat(b, c);
    printLine2(b);     
  }

  cwTimeout = 0;
  keyDown = 0;
  stopTx();

  printLineF2(F("Calibration set!"));
  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  setFrequency(frequency);    
  updateDisplay();

  while(btnDown())
    active_delay(50);
  active_delay(100);
}

int menuSetupCalibration(int btn){
  int knob = 0;
  int32_t prev_calibration;
   
  if (!btn){
    printLineF2(F("Setup:Calibrate\x7E"));
    return 0;
  }

  printLineF1(F("Press PTT & tune"));
  printLineF2(F("to exactly 10 MHz"));
  active_delay(2000);
  calibrateClock();
}

void printCarrierFreq(unsigned long freq){

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(freq, b, DEC);
  
  strncat(c, b, 2);
  strcat(c, ".");
  strncat(c, &b[2], 3);
  strcat(c, ".");
  strncat(c, &b[5], 1);
  printLine2(c);    
}

void
printFreq(unsigned long freq)
{
  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));
  ultoa(freq, b, DEC);
  printLine2(b);
}

void menuSetupCarrier(int btn){
  int knob = 0;
  unsigned long prevCarrier;
   
  if (!btn){
      printLineF2(F("Setup:BFO      \x7E"));
    return;
  }

  prevCarrier = usbCarrier;
  printLineF1(F("Tune to best Signal"));  
  printLineF2(F("Press to confirm. "));
  active_delay(1000);

  usbCarrier = INIT_USB_FREQ;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  //disable all clock 1 and clock 2 
  while (!btnDown()){
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; //don't update the frequency or the display
      
    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);
    
    active_delay(100);
  }

  printLineF2(F("Carrier set!    "));
  EEPROM.put(USB_CAL, usbCarrier);
  active_delay(1000);
  
  si5351bx_setfreq(0, usbCarrier);          
  setFrequency(frequency);    
  updateDisplay();
  printLineF2(F(""));
  menuOn = 0; 
}

// Calibrate the IF frequency
void menuSetupFreqIF(int btn){
  int knob = 0;
  unsigned long prevIF;
   
  if (!btn){
      printLineF2(F("Setup:IF      \x7E"));
    return;
  }

  prevIF = firstIF;
  printLineF1(F("Tune to loudest signal"));
  printLineF2(F("Press to confirm. "));
  active_delay(1000);

  setFrequency(frequency);
  printFreq(firstIF);

  //disable all clock 1 and clock 2 
  while (!btnDown()){
    knob = enc_read();

    if (knob > 0)
      firstIF += 50;
    else if (knob < 0)
      firstIF -= 50;

    checkPTT();

    if (knob == 0)
      continue; //don't update the frequency or the display
      
    setFrequency(frequency);
    printFreq(firstIF);
    
    active_delay(100);
  }

  printLineF2(F("IF set!    "));
  
  EEPROM.put(CAL_FIRST_IF_FREQ, firstIF);
  active_delay(1000);
  setFrequency(frequency);    
  updateDisplay();
  printLineF2(F(""));
  menuOn = 0; 
}

void menuSetupCwTone(int btn){
  int knob = 0;
  int prev_sideTone;
     
  if (!btn){
    printLineF2(F("Setup:CW Tone  \x7E"));
    return;
  }

  prev_sideTone = sideTone;
  printLineF1(F("Tune CW tone"));
  printLineF2(F("PTT to confirm. "));
  active_delay(1000);
  tone(CW_TONE, sideTone);

  //disable all clock 1 and clock 2 
  while (digitalRead(PTT) == HIGH && !btnDown())
  {
    knob = enc_read();

    if (knob > 0 && sideTone < 2000)
      sideTone += 10;
    else if (knob < 0 && sideTone > 100 )
      sideTone -= 10;
    else
      continue; //don't update the frequency or the display
        
    tone(CW_TONE, sideTone);
    itoa(sideTone, b, 10);
    printLine2(b);

    checkCAT();
    active_delay(20);
  }
  noTone(CW_TONE);
  //save the setting
  if (digitalRead(PTT) == LOW){
    printLineF2(F("Sidetone set!    "));
    EEPROM.put(CW_SIDETONE, sideTone);
    active_delay(2000);
  }
  else
    sideTone = prev_sideTone;
    
  printLineF2(F(""));
  updateDisplay();
  menuOn = 0;
}

void menuSetupCwDelay(int btn){
  int knob = 0;
  int prev_cw_delay;

  if (!btn){
    printLineF2(F("Setup:CW Delay \x7E"));
    return;
  }

  active_delay(500);
  prev_cw_delay = cwDelayTime;
  cwDelayTime = getValueByKnob(10, 1000, 50,  cwDelayTime, "CW Delay>", " msec");

  printLineF1(F("CW Delay Set!"));
  printLineF2(F(""));
  active_delay(500);
  updateDisplay();
  menuOn = 0;
}

void menuSetupKeyer(int btn){
  int tmp_key, knob;
  
  if (!btn){
    if (!Iambic_Key)
      printLineF2(F("Setup:CW(Hand)\x7E"));
    else if (keyerControl & IAMBICB)
      printLineF2(F("Setup:CW(IambA)\x7E"));
    else 
      printLineF2(F("Setup:CW(IambB)\x7E"));   
    return;
  }
  
  active_delay(500);

  if (!Iambic_Key)
    tmp_key = 0; //hand key
  else if (keyerControl & IAMBICB)
    tmp_key = 2; //Iambic B
  else 
    tmp_key = 1;
 
  while (!btnDown())
  {
    knob = enc_read();
    if (knob < 0 && tmp_key > 0)
      tmp_key--;
    if (knob > 0)
      tmp_key++;

    if (tmp_key > 2)
      tmp_key = 0;
      
    if (tmp_key == 0)
      printLineF1(F("Hand Key?"));
    else if (tmp_key == 1)
      printLineF1(F("Iambic A?"));
    else if (tmp_key == 2)  
      printLineF1(F("Iambic B?")); 
  }

  active_delay(500);
  if (tmp_key == 0)
    Iambic_Key = false;
  else if (tmp_key == 1){
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (tmp_key == 2){
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
  
  EEPROM.put(CW_KEY_TYPE, tmp_key);
  
  printLineF1(F("Keyer Set!"));
  active_delay(600);
  printLineF1(F(""));

  //Added KD8CEC
  printLineF2(F(""));
  updateDisplay(); 
  menuOn = 0;
}

void menuReadADC(int btn){
  int adc;
  char a = 0, al; // Go from A0 -> A7
  int knob;

  if (!btn){
    printLineF2(F("6:Setup>Read ADC>"));
    return;
  }
  delay(500);

  while (!btnDown()){
    // Assemble the display string, showing the A line and value
    c[0] = 0;
    strcat(c, "A");
    itoa(a, b, 10);
    strcat(c, b);
    strcat(c, " = ");

    // Map A line to value, yeah, I wish it were easier?
    switch (a) {
      case 0: al = A0; break;
      case 1: al = A1; break;
      case 2: al = A2; break;
      case 3: al = A3; break;
      case 4: al = A4; break;
      case 5: al = A5; break;
      case 6: al = A6; break;
      case 7: al = A7; break;
    }
    adc = analogRead(al);
    itoa(adc, b, 10);
    strcat(c, b);
    printLine1(c);

    // Read the encoder, see if we need to change the ADC
    knob = enc_read();
    if ((knob > 4) && (a < 7)) {
      a++;
      delay(100);
    }
    if ((knob < -4) && (a > 0)) {
      a--;
      delay(100);
    }
  }

  printLineF1(F(""));

  // Debounce the menu select
  delay(500);
  updateDisplay();
}

void doMenu(){
  int select=0, i,btnState;

  //wait for the button to be raised up
  while(btnDown())
    active_delay(50);
  active_delay(50);  //debounce
  
  menuOn = 2;
  
  while (menuOn){
    i = enc_read();
    btnState = btnDown();

    if (i > 0){
      if (modeCalibrate && select + i < 160)
        select += i;
      if (!modeCalibrate && select + i < 80)
        select += i;
    }
    if (i < 0 && select - i >= 0)
      select += i;      //caught ya, i is already -ve here, so you add it

    if (select < 10)
      menuBand(btnState);
    else if (select < 20)
      menuRitToggle(btnState);
    else if (select < 30)
      menuVfoToggle(btnState);
    else if (select < 40)
      menuSidebandToggle(btnState);
    else if (select < 50)
      menuSplitToggle(btnState);
    else if (select < 60)
      menuCWSpeed(btnState);
    else if (select < 70)
      select += menuSetup(btnState);
    else if (select < 80 && !modeCalibrate)
      menuExit(btnState);
    else if (select < 90 && modeCalibrate)
      menuSetupCalibration(btnState);   //crystal
    else if (select < 100 && modeCalibrate)
      menuSetupCarrier(btnState);       //lsb
    else if (select < 110 && modeCalibrate)
      menuSetupCwTone(btnState);
    else if (select < 120 && modeCalibrate)
      menuSetupCwDelay(btnState);
    else if (select < 130 && modeCalibrate)
      menuReadADC(btnState);
    else if (select < 140 && modeCalibrate)
        menuSetupKeyer(btnState);
    else if (select < 150 && modeCalibrate)
        menuSetupFreqIF(btnState);
    else
      menuExit(btnState);  
  }

  //debounce the button
  while(btnDown())
    active_delay(50);
  active_delay(50);

  checkCAT();
}
