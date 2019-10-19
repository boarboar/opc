#include <SPI.h>

#include "TFT_ILI9341.h"
  
#define WS_SCREEN_SIZE_X  320
#define WS_SCREEN_SIZE_Y  240
#define WS_SCREEN_STAT_LINE_Y  224
#define WS_SCREEN_TIME_LINE_Y  0
#define WS_SCREEN_TEMP_LINE_Y  60
#define WS_SCREEN_TEMP_LINE_PADDING 8
#define WS_SCREEN_TITLE_X (WS_SCREEN_SIZE_X-4*WS_CHAR_DEF_SIZE*FONT_SPACE)

#define WS_CHAR_DEF_SIZE 2

TFT Tft;

void setup()
{
  delay(100);
    
  Tft.TFTinit();
  Tft.setOrientation(LCD_LANDSCAPE);
  
  lcd_defaults();
  
  dispStat("INIT CLK"); delay(200);  
}

void loop()
{
 
}

uint16_t _lp_vpos=0; // make 8bit, and prop to char size?
uint16_t _lp_hpos=0;  // make 8bit, and prop to char size?

inline uint16_t line_getpos() { return _lp_vpos; }
inline uint16_t line_getposx() { return _lp_hpos; }

inline void line_init() {
  _lp_vpos=_lp_hpos=0;
}

void line_print(const char* pbuf) {
  line_printn(pbuf);
  _lp_hpos=0;
  _lp_vpos+=FONT_Y*Tft.getSize();
}

inline void line_setpos(uint16_t x, uint16_t y) {
  _lp_hpos=x; _lp_vpos=y;
}

inline void line_setcharpos(uint8_t x) {
  _lp_hpos=(uint16_t)x*Tft.getSize()*FONT_SPACE;
}

void line_printn(const char* pbuf) {
  _lp_hpos = Tft.drawString(pbuf ,_lp_hpos, _lp_vpos);
}

void dispStat(const char *pbuf) {
  lcd_defaults();
  line_setpos(0, WS_SCREEN_STAT_LINE_Y); line_printn(pbuf); 
}

void lcd_defaults() {
  Tft.setBgColor(BLACK);
  Tft.setFgColor(GREEN);
  Tft.setSize(WS_CHAR_DEF_SIZE);
  Tft.setOpaq(LCD_OPAQ);
}
 
