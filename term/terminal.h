
#ifndef Terminal_h
#define Terminal_h

#include "TFT_ILI9341.h"

/*
#define WS_ORIENT    LCD_LANDSCAPE
#define WS_SCREEN_SIZE_X  320
#define WS_SCREEN_SIZE_Y  240
*/

#define WS_ORIENT    LCD_PORTRAIT
#define WS_SCREEN_SIZE_X  240
#define WS_SCREEN_SIZE_Y  320

#define WS_BG_COLOR  BLACK
#define WS_FG_COLOR  GREEN
//#define WS_FG_COLOR  WHITE
#define WS_FG_COLOR_CTRL  YELLOW

#define WS_CHAR_DEF_SIZE 1
#define WS_CHAR_S_X  (WS_CHAR_DEF_SIZE*FONT_SPACE)
#define WS_CHAR_N_X   (WS_SCREEN_SIZE_X/WS_CHAR_S_X)
#define WS_CHAR_S_Y  (WS_CHAR_DEF_SIZE*FONT_Y)
#define WS_CHAR_N_Y   (WS_SCREEN_SIZE_Y/WS_CHAR_S_Y)

#define WS_TAB_IDENT  2

//#define WS_F_CUR_ON    0x01
#define WS_F_CUR_VIS   0x02
//#define WS_F_CUR_SUPP   0x04
//#define WS_F_CUR_IGNORE 0x08

#define ESC_CHAR '\x1b'

#define ESC_STATE_0   1
#define ESC_STATE_ESC 2
#define ESC_STATE_BR  3
#define ESC_STATE_V0  4
#define ESC_STATE_V1  5

class LCDTerminal {
public:
  void init();
  void lcd_defaults();
//  inline void cursorCtrlOff() {_flags |= WS_F_CUR_IGNORE;}
//  inline void cursorCtrlOn() {_flags &= ~WS_F_CUR_IGNORE;}
//  inline void cursorOn() {_flags |= WS_F_CUR_ON; showCursor();}
//  inline void cursorOff() {hideCursor(); _flags &= ~WS_F_CUR_ON;}
  inline void cursorBlink() {if(_flags & WS_F_CUR_VIS) hideCursor(); else showCursor();} 
  void printc(char c);
  void prints(const char *s);
  inline void println(const char *s) {prints(s); printc('\n');}
  void scroll();
  void showCursor();
  void hideCursor();
  inline void setDataColor() {Tft.setFgColor(WS_FG_COLOR);}
  inline void setCtrlColor() {Tft.setFgColor(WS_FG_COLOR_CTRL);}
  inline void eraseEOL() {Tft.setFillColor(LCD_BG); Tft.fillScreen((INT16U)(_x_pos)*WS_CHAR_S_X, (INT16U)(_x_eol_pos)*WS_CHAR_S_X-1, (INT16U)(_yeff)*WS_CHAR_S_Y, (INT16U)(_yeff+1)*WS_CHAR_S_Y);}
protected:  
  TFT Tft;
  uint8_t _y_pos; 
  uint8_t _x_pos;  // cursor pos
  uint8_t _x_eol_pos;  
  uint8_t _y_scroll;  
  uint8_t _yeff;
  uint8_t _flags;
  char  _prev_chr;  
  uint8_t _esc_state;
  uint8_t _esc_val[2];
  
  //uint8_t _line_len[WS_CHAR_N_Y];
  
  void advance_y();
  char esc_cmd(char c);

};

#endif
