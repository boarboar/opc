
#include "terminal.h"

void LCDTerminal::init() {
  Tft.TFTinit();
  Tft.setOrientation(LCD_LANDSCAPE);  
  lcd_defaults();
  _x_pos = _y_pos = 0;
  _y_scroll=0;
  //line_print("WELCOME"); 
}

void LCDTerminal::lcd_defaults() {
  Tft.setBgColor(BLACK);
  Tft.setFgColor(GREEN);
  Tft.setSize(WS_CHAR_DEF_SIZE);
  Tft.setOpaq(LCD_TRANSP);
  Tft.setupScrollArea(WS_SCREEN_SIZE_Y, WS_CHAR_S_Y, 0);
}

void LCDTerminal::print(char c) {
  //if(c=='\n')
  Tft.drawCharLowRAM(c, (INT16U)_x_pos*WS_CHAR_S_X, (INT16U)_y_pos*WS_CHAR_S_Y);
  if(++_x_pos >= WS_CHAR_N_X) {
    _x_pos = 0;
    if(++_y_pos >= WS_CHAR_N_Y) {
      // scroll - TODO
      --_y_pos;
    }
  }
}

void LCDTerminal::scroll() {
  if(++_y_scroll >= WS_CHAR_N_Y) _y_scroll=0;
  Tft.scrollAddress((INT16U)_y_scroll*WS_CHAR_S_Y);
}

/*
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

void lcd_defaults() {
  Tft.setBgColor(BLACK);
  Tft.setFgColor(GREEN);
  Tft.setSize(WS_CHAR_DEF_SIZE);
  Tft.setOpaq(LCD_OPAQ);
}
*/ 
