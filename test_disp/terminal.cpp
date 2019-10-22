
#include "terminal.h"

void LCDTerminal::init() {
  Tft.TFTinit();
  Tft.setOrientation(WS_ORIENT);  
  lcd_defaults();
  _x_pos = _y_pos = 0;
  _y_scroll=0;
  //line_print("WELCOME"); 
}

void LCDTerminal::lcd_defaults() {
  Tft.setBgColor(WS_BG_COLOR);
  Tft.setFgColor(WS_FG_COLOR);
  Tft.setSize(WS_CHAR_DEF_SIZE);
  Tft.setOpaq(LCD_TRANSP);
  Tft.setupScrollArea(WS_SCREEN_SIZE_Y, 0, 0);
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
  //INT16U bot;
  if(++_y_scroll > WS_CHAR_N_Y) _y_scroll=0;
  Tft.scrollAddress((INT16U)(_y_scroll)*WS_CHAR_S_Y);
  Tft.setBgColor(WS_BG_COLOR);
  Tft.setFillColor(LCD_BG); 
  //bot = (WS_SCREEN_SIZE_Y+_y_scroll*WS_CHAR_S_Y)%WS_SCREEN_SIZE_Y
  Tft.fillScreen(0, WS_SCREEN_SIZE_X-1, (INT16U)(_y_scroll-1)*WS_CHAR_S_Y, (INT16U)_y_scroll*WS_CHAR_S_Y);
  Tft.drawCharLowRAM('>', 0, (INT16U)(_y_scroll-1)*WS_CHAR_S_Y);
}

