
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

#define WS_CHAR_DEF_SIZE 2
#define WS_CHAR_S_X  (WS_CHAR_DEF_SIZE*FONT_SPACE)
#define WS_CHAR_N_X   (WS_SCREEN_SIZE_X/WS_CHAR_S_X)
#define WS_CHAR_S_Y  (WS_CHAR_DEF_SIZE*FONT_Y)
#define WS_CHAR_N_Y   (WS_SCREEN_SIZE_Y/WS_CHAR_S_Y)

#define WS_TAB_IDENT  2

class LCDTerminal {
public:
  void init();
  void lcd_defaults();
  void print(char c);
  void print(const char *s);
  inline void println(const char *s) {print(s); print('\n');}
  void scroll();
protected:  
  TFT Tft;
  uint8_t _y_pos; 
  uint8_t _x_pos;  
  uint8_t _y_scroll;  
  uint8_t _yeff;
  void advance_y();
};

#endif
