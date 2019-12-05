
#include "terminal.h"

void LCDTerminal::init() {
  _x_pos = _y_pos = 0;
  _y_scroll = 0;
  _flags = 0;
  Tft.TFTinit();
  Tft.setOrientation(WS_ORIENT);  
  lcd_defaults();
  showCursor();
  //line_print("WELCOME"); 
}

void LCDTerminal::lcd_defaults() {
  Tft.setBgColor(WS_BG_COLOR);
  Tft.setFgColor(WS_FG_COLOR);
  Tft.setSize(WS_CHAR_DEF_SIZE);
  Tft.setOpaq(LCD_TRANSP);
  Tft.setFillColor(LCD_BG);
  Tft.setupScrollArea(WS_SCREEN_SIZE_Y, 0, 0);
  _flags |= WS_F_CUR_ON;
}

void LCDTerminal::prints(const char *s) {
  if(_flags & WS_F_CUR_ON) {
    _flags |= WS_F_CUR_SUPP;
    cursorOff();
  }
  setCtrlColor();
  const char *p = s;
  while(*p) printc(*p++);
  setDataColor();
  if(_flags & WS_F_CUR_SUPP) {
    _flags |= WS_F_CUR_ON;
    cursorOn();
  }
}
  
void LCDTerminal::printc(char c, bool cctrl) {  
  if(cctrl) hideCursor();
  switch(c) {
    case '\n' :
    case '\r' :
      advance_y();
      break;
    case '\t' :  
      _x_pos+=WS_TAB_IDENT;
      if(_x_pos >= WS_CHAR_N_X) advance_y();
      break;
    case '\b' :
      if(_x_pos > 0) {
        _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1; 
        Tft.setFillColor(LCD_BG);
        Tft.fillScreen((INT16U)(_x_pos-1)*WS_CHAR_S_X, (INT16U)(_x_pos)*WS_CHAR_S_X-1, (INT16U)(_yeff)*WS_CHAR_S_Y, (INT16U)(_yeff+1)*WS_CHAR_S_Y);
        _x_pos--;
      }
      break;
    default:
      _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1;
      Tft.drawCharLowRAM(c, (INT16U)_x_pos*WS_CHAR_S_X, (INT16U)_yeff*WS_CHAR_S_Y);
      if(++_x_pos >= WS_CHAR_N_X) advance_y();
  }
  if(cctrl) showCursor();
}

void LCDTerminal::advance_y() {
  _x_pos = 0;
  if(++_y_pos>=WS_CHAR_N_Y) {
    _y_pos=WS_CHAR_N_Y;
    scroll();
  }
}
  
void LCDTerminal::scroll() {
  if(++_y_scroll > WS_CHAR_N_Y) {
    _y_scroll=1;
  }
  Tft.scrollAddress((INT16U)(_y_scroll)*WS_CHAR_S_Y); 
  Tft.setFillColor(LCD_BG);
  Tft.fillScreen(0, WS_SCREEN_SIZE_X-1, (INT16U)(_y_scroll-1)*WS_CHAR_S_Y, (INT16U)_y_scroll*WS_CHAR_S_Y);
}

void LCDTerminal::showCursor() {
 if(_flags&WS_F_CUR_ON && !(_flags&WS_F_CUR_VIS)) 
  {
    //Serial.println("ON");
    _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1; 
    Tft.setFillColor(LCD_FG);
    Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X+2, (INT16U)(_x_pos+1)*WS_CHAR_S_X-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2);
    _flags |= WS_F_CUR_VIS;
  }
}

void LCDTerminal::hideCursor() {
  if( _flags&WS_F_CUR_ON && _flags&WS_F_CUR_VIS) 
  {
    //Serial.println("OFF");
    _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1; 
    Tft.setFillColor(LCD_BG);
    Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X+2, (INT16U)(_x_pos+1)*WS_CHAR_S_X-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-1);
    _flags &= ~WS_F_CUR_VIS;
  }
}
