#ifndef __UBITX_UI_H__
#define __UBITX_UI_H__

extern int btnDown(void);
extern void initMeter(void);
extern void drawMeter(uint8_t needle);
extern void printLine(char linenmbr, const char *c);
extern void printLineF(char linenmbr, const __FlashStringHelper *c);


#define printLine1(c) printLine(1, (c))
#define printLine2(c) printLine(0, (c))

#define printLineF1(c) printLineF(1, (c))
#define printLineF2(c) printLineF(0, (c))

extern void updateDisplay(void);
extern void updateMeterDisplay(void);
extern void clearMeterDisplay(void);
extern byte enc_state (void);
extern int enc_read(void);

#endif
