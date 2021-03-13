#ifndef __UBITX_UI_H__
#define __UBITX_UI_H__

extern int btnDown(void);
extern void initMeter(void);
extern void drawMeter(uint8_t needle);
extern void printLine(char linenmbr, const char *c);
extern void printLine1(const char *c);
extern void printLine2(const char *c);
extern void updateDisplay(void);
extern void updateMeterDisplay(void);
extern void clearMeterDisplay(void);
extern byte enc_state (void);
extern int enc_read(void);

#endif
