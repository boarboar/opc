
#include "terminal.h"

void LCDTerminal::init() {
  _y_pos = _yeff = 0;
  _x_pos = _x_eol_pos = 0;
  _y_scroll = 0;
  _flags = 0;
  _prev_chr = 0;
  _esc_state = ESC_STATE_0;
  _esc_val = 0;
  Tft.TFTinit();
  Tft.setOrientation(WS_ORIENT);  
  lcd_defaults();
  showCursor();
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
        if(c=='[') { 
          _esc_state = ESC_STATE_BR;
          _esc_val = 0;
          c=0;
        } else _esc_state = ESC_STATE_0;
        break;
      case ESC_STATE_BR:
        if(isdigit(c)) {
          _esc_val = _esc_val*10+(c-'0');
          c=0;
          break;
        }
        else switch(c) {
          case 'K' : // fill until EOL
          /*
            Tft.setFillColor(LCD_BG);
            Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X, (INT16U)(_x_eol_pos)*WS_CHAR_S_X-1, (INT16U)(_yeff)*WS_CHAR_S_Y, (INT16U)(_yeff+1)*WS_CHAR_S_Y);
            */
            eraseEOL();
            c=0;
            _esc_state = ESC_STATE_0;
            break;
          case 'D' : 
            c=0;  
            if(_esc_val == 0) _esc_val = 1;
            if(_esc_val > _x_pos) _esc_val = _x_pos;
            _x_pos -= _esc_val;
            _esc_state = ESC_STATE_0;
            break;
          default: _esc_state = ESC_STATE_0;
        }  
      default: _esc_state = ESC_STATE_0;
    }  
  }
  
  if(c==0) return;
  
  if(cctrl) hideCursor();
  switch(c) {
    case '\n' : //lf
      if(_prev_chr=='\r') break;  // ignore CR LF
    case '\r' : //cr    
      advance_y();
      break;
    case '\t' :  
      _x_pos+=WS_TAB_IDENT;
      _x_eol_pos+=WS_TAB_IDENT;
      if(_x_eol_pos >= WS_CHAR_N_X) advance_y();
      break;
    case '\b' :
      if(_x_pos > 0) _x_pos--;     
      break;
    case ESC_CHAR :
      _esc_state=ESC_STATE_ESC;
      break;
    default:
      if(_x_pos<_x_eol_pos) eraseEOL();
      Tft.drawCharLowRAM(c, (INT16U)_x_pos*WS_CHAR_S_X, (INT16U)_yeff*WS_CHAR_S_Y);
      ++_x_pos;
      ++_x_eol_pos;
      if(_x_eol_pos >= WS_CHAR_N_X) advance_y();
  }
  _prev_chr=c;
  if(cctrl) showCursor();
}

void LCDTerminal::advance_y() {
  if(++_y_pos>=WS_CHAR_N_Y) {
    _y_pos=WS_CHAR_N_Y;    
    scroll();
  }
  _yeff = _y_pos<WS_CHAR_N_Y ? _y_pos : _y_scroll-1;
  _x_pos = 0;
  _x_eol_pos = 0;
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
    Tft.setFillColor(LCD_FG);
    Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X+2, (INT16U)(_x_pos+1)*WS_CHAR_S_X-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2);
    _flags |= WS_F_CUR_VIS;
  }
}

void LCDTerminal::hideCursor() {
  if( _flags&WS_F_CUR_ON && _flags&WS_F_CUR_VIS) 
  {
    Tft.setFillColor(LCD_BG);
    Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X+2, (INT16U)(_x_pos+1)*WS_CHAR_S_X-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-2, (INT16U)(_yeff+1)*WS_CHAR_S_Y-1);
    _flags &= ~WS_F_CUR_VIS;
  }
}
