
#include "terminal.h"

void LCDTerminal::init() {
  _x_pos = _y_pos = 0;
  _y_scroll = 0;
  _flags = 0;
  _prev_chr = 0;
  _esc_state = ESC_STATE_0;
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
  
  if(_esc_state>ESC_STATE_0) { // handle ESQ seqs
    switch(_esc_state) {
      case ESC_STATE_ESC:
        if(c=='[') c = '^'; // to be continued
        _esc_state = 0;
      default: _esc_state = 0;
    }  
  }
  
  if(cctrl) hideCursor();
  switch(c) {
    case '\n' : //lf
      if(_prev_chr=='\r') break;  // ignore CR LF
    case '\r' : //cr    
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
    case ESC_CHAR :
      //c =  '~';  // ESC
      _esc_state=ESC_STATE_ESC;
      break;
    //case '[':
    //  if(_prev_chr==ESC_CHAR) c = '^';  
    default:
      _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1;
      Tft.drawCharLowRAM(c, (INT16U)_x_pos*WS_CHAR_S_X, (INT16U)_yeff*WS_CHAR_S_Y);
      if(++_x_pos >= WS_CHAR_N_X) advance_y();
  }
  _prev_chr=c;
  if(cctrl) showCursor();
}

void LCDTerminal::advance_y() {
  if(++_y_pos>=WS_CHAR_N_Y) {
    _y_pos=WS_CHAR_N_Y;
    if(_x_pos>=WS_CHAR_N_X) _x_pos = WS_CHAR_N_X-1;
    scroll();
  }
  _x_pos = 0;
}
  
void LCDTerminal::scroll() {
  if(++_y_scroll > WS_CHAR_N_Y) {
    _y_scroll=1;
  }
  Tft.scrollAddress((INT16U)(_y_scroll)*WS_CHAR_S_Y); 
  Tft.setFillColor(LCD_BG);
  Tft.fillScreen(0, WS_SCREEN_SIZE_X-1, (INT16U)(_y_scroll-1)*WS_CHAR_S_Y, (INT16U)_y_scroll*WS_CHAR_S_Y);
  //Tft.fillScreen(0, (INT16U)(_x_pos)*WS_CHAR_S_X-1, (INT16U)(_y_scroll-1)*WS_CHAR_S_Y, (INT16U)_y_scroll*WS_CHAR_S_Y);
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
