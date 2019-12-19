
#include <SPI.h>
#include "terminal.h"

// 2552 USI
// 2553 USCI

LCDTerminal term;

#define KB_LED RED_LED

const byte COL_COUNT = 10;
const byte ROW_COUNT = 4;

#define latchPin P2_3
#define dataPin P2_4
#define clockPin P2_5

const uint16_t rowPins[ROW_COUNT] = {P2_2, P2_1, P2_0, P1_4}; //can't use P1_5 - spi clk

#define  KEY_COUNT_BYTES ((COL_COUNT*ROW_COUNT-1)/8+1)
uint8_t  key_dn[KEY_COUNT_BYTES] = {0};
uint8_t  key_in[KEY_COUNT_BYTES] = {0};

#define  SCAN_CODE(R, C) ((R)*COL_COUNT+(C))
#define  IS_KEY_DN(S)  (key_dn[(S)/8] & (1<<((S)%8)))    
#define  IS_KEY_IN(S)  (key_in[(S)/8] & (1<<((S)%8)))
#define  SET_KEY_DN(S)  (key_dn[(S)/8] |= (1<<((S)%8)))
#define  CLR_KEY_DN(S)  (key_dn[(S)/8] &= ~(1<<((S)%8)))
#define  SET_KEY_IN(S)  (key_in[(S)/8] |= (1<<((S)%8)))
#define  CLR_KEY_IN(S)  (key_in[(S)/8] &= ~(1<<((S)%8)))

#define KEY_SHIFT   0xff
#define KEY_FN      0xfe
#define KEY_ESC_LAST   0xfd
#define KEY_ESC_R   0xfd  // escape seq
#define KEY_ESC_D   0xfc  // escape seq
#define KEY_ESC_U   0xfb  // escape seq
#define KEY_ESC_L   0xfa  // escape seq
#define KEY_ESC_FIRST   0xfa
#define KEY_SERIAL_SPEED   0xf9
#define KEY_SERIAL_ECHO    0xf8
#define KEY_SERIAL_XCTRL   0xf7
#define KEY_CTRL_C  0x3
#define KEY_CTRL_D  0x4
#define KEY_ESC     0x1b
#define KEY_DEL     0x7f

const uint8_t keymap[ROW_COUNT][COL_COUNT] = {
{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
{'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
{'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\r'},
{KEY_SHIFT, KEY_FN, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ' '}
};


const uint8_t keymap_shift[ROW_COUNT][COL_COUNT] = 
{
{'!','@','#','$','%','^','&','*','(',')'},
{'Q','W','E','R','T','Y','U','I','O','P'},
{'A','S','D','F','G','H','J','K','L','\n'},
{0, 0, 'Z','X','C','V','B','N','M',' '}
};

const uint8_t keymap_fn[ROW_COUNT][COL_COUNT] = {
{KEY_ESC,  KEY_CTRL_C, KEY_CTRL_D, 0, 0, KEY_ESC_L, KEY_ESC_U, KEY_ESC_D, KEY_ESC_R, '\b'},  //  l, u, d, r, bsp
{'\t', '~', 0, 0, '-', '_', '=', '+', '\\', '|'},
{KEY_DEL, KEY_SERIAL_XCTRL, ';', ':', '"', '\'', '[', '{', ']', '}'},
{0,0,KEY_SERIAL_SPEED, KEY_SERIAL_ECHO,',','<', '.', '>', '/', '?'}
};

#define SERIAL_RATES_N  5
const uint32_t serial_rates[]={9600, 19200, 38400, 57600, 115200};
const char *serial_rates_descr[]={"[9600]", "[19200]", "[38400]", "[57600]", "[115200]"};

#define F_SHIFT_PRES  0x01
#define F_FN_PRES     0x02
#define F_SER_ECHO_ON  0x10
#define F_SER_XCTRL_ON  0x20

uint8_t cursor_cnt=0;
uint8_t flags=F_SER_ECHO_ON;
char key_pressed=0;
uint8_t rep = 0;
uint8_t serial_rate_idx=0; //9600
uint32_t t;

#define SCAN_DELAY 20
#define REP_COUNT  20
#define CURSOR_COUNT  16

#define EMIT(K) { Serial.print((K)); if(flags&F_SER_ECHO_ON) term.printc((K)); }  
#define PRINT_ECHO_STATUS() {term.println((flags&F_SER_ECHO_ON) ? "[ECHO ON]" : "[ECHO OFF]");}
#define PRINT_XCTRL_STATUS() {term.println((flags&F_SER_XCTRL_ON) ? "[XONOFF ON]" : "[XONOFF OFF]");}

#define CODE_XOFF  '\x13'
#define CODE_XON   '\x11' 

#define EMIT_XON() {if(flags&F_SER_XCTRL_ON) Serial.print(CODE_XON);}
#define EMIT_XOFF() {if(flags&F_SER_XCTRL_ON) Serial.print(CODE_XOFF);}


void setup() {                
  // initialize the digital pin as an output.
  digitalWrite(KB_LED, LOW); 
  pinMode(KB_LED, OUTPUT); 
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
 
  for(int row=0; row<ROW_COUNT; row++) pinMode(rowPins[row], INPUT_PULLUP);  
  
  digitalWrite(KB_LED, HIGH);
  Serial.begin(9600);  
  EMIT_XOFF();
  term.init();  
  delay(10);
  while(Serial.available()>0) Serial.read(); // clear input
  term.prints("TERM v1.01 "); 
  term.prints(serial_rates_descr[serial_rate_idx]);
  PRINT_ECHO_STATUS();
  PRINT_XCTRL_STATUS();
  
  digitalWrite(KB_LED, LOW);
  t=millis();
  EMIT_XON();
}

void loop() {
  if(Serial.available()>0) {
    uint16_t cc=0;  
    term.hideCursor();
    term.cursorCtrlOff();
    while(Serial.available()>0 && cc++<400) { // limit to 400 chars at once to give kbhandler a chance
      EMIT_XOFF();
      byte b = Serial.read();       
      term.printc((char)b);
      EMIT_XON();
      //Serial.print((char)b);
      // if(cc>16) Serial.print(CODE_XOFF); // try this
    }
    term.showCursor();
    term.cursorCtrlOn();
  }
  if(millis()<t) { // wraparound
    t=millis();
    return;
  }
  if(millis()>=t+SCAN_DELAY) {
    EMIT_XOFF();
    key_loop();
    t=millis();
    if(++cursor_cnt>=CURSOR_COUNT) {
      cursor_cnt=0;
      term.cursorBlink();
    }
    EMIT_XON();
  }
  
}

inline void key_loop() {
  uint16_t u=1;
  for (uint8_t col = 0; col < COL_COUNT; col++)   
  {
    uint8_t s, rval;
    
    digitalWrite(latchPin, HIGH); //Pull latch HIGH to send data
    shiftOut(dataPin, clockPin, MSBFIRST, (~u)>>8); //Send the data HIBYTE
    shiftOut(dataPin, clockPin, MSBFIRST, (~u)&0xFF); //Send the data LOBYTE
    digitalWrite(latchPin, LOW); // Pull latch LOW to stop sending data
    
    for(uint8_t row=0; row<ROW_COUNT; row++) { 
      rval=digitalRead(rowPins[row]); // 16_t ???
      s = SCAN_CODE(row, col);
      if(rval==LOW) { // DN
        if(IS_KEY_DN(s)) {
          if(IS_KEY_IN(s)) CLR_KEY_IN(s); // clr attempt to up 
        }
        else {
          if(IS_KEY_IN(s)) { // do key logic
            CLR_KEY_IN(s);
            SET_KEY_DN(s);
            
            switch(keymap[row][col]) {              
              case KEY_SHIFT: flags|=F_SHIFT_PRES; break;
              case KEY_FN: flags|=F_FN_PRES; break;
              default:
                char key = 
                  flags&F_SHIFT_PRES ? keymap_shift[row][col] :
                  flags&F_FN_PRES ? keymap_fn[row][col] :
                  keymap[row][col];
                if((uint8_t)key==KEY_SERIAL_SPEED) {
                  serial_rate_idx = (serial_rate_idx+1)%SERIAL_RATES_N; 
                  Serial.flush();
                  Serial.begin(serial_rates[serial_rate_idx]); 
                  term.println(serial_rates_descr[serial_rate_idx]);
                  key=0;
                } else if((uint8_t)key==KEY_SERIAL_ECHO) {
                  if(flags&F_SER_ECHO_ON) {
                    flags&=~F_SER_ECHO_ON;
                  } else {
                    flags|=F_SER_ECHO_ON;
                  }
                  PRINT_ECHO_STATUS();
                  key=0;
                } else if((uint8_t)key==KEY_SERIAL_XCTRL) {
                  if(flags&F_SER_XCTRL_ON) {
                    flags&=~F_SER_XCTRL_ON;
                  } else {
                    flags|=F_SER_XCTRL_ON;
                  }
                  PRINT_XCTRL_STATUS();
                  key=0;
                }
                if(key) {
                  key_pressed = key;
                  rep=0;
                  if((uint8_t)key <= KEY_ESC_LAST && (uint8_t)key >= KEY_ESC_FIRST) {
                    /*
                    Serial.print('\x1b'); //ESC
                    if(flags&F_SER_ECHO_ON) term.printc('\x1b');
                    Serial.print('[');
                    if(flags&F_SER_ECHO_ON) term.printc('[');
                    */
                    // ESC
                    EMIT('\x1b');
                    EMIT('[');
                    switch((uint8_t)key) {
                      case KEY_ESC_L:
                        key='D';
                        break;
                      case KEY_ESC_R:
                        key='C';
                        break;
                      case KEY_ESC_U:
                        key='A';
                        break;
                      case KEY_ESC_D:
                        key='B';
                        break;  
                      default:;                      
                    }
                  }
                  //Serial.print(key);
                  //if(flags&F_SER_ECHO_ON) term.printc(key);          
                  EMIT(key);
                }
            }
            digitalWrite(KB_LED, HIGH);             
          }
          else {
            SET_KEY_IN(s); // suspect to dn
          }
        }
      } 
      else { // key_up
        if(IS_KEY_DN(s)) {
          if(IS_KEY_IN(s)) { // release key
            CLR_KEY_IN(s);
            CLR_KEY_DN(s);        
            switch(keymap[row][col]) {
              case KEY_SHIFT: flags&=~F_SHIFT_PRES; break;
              case KEY_FN: flags&=~F_FN_PRES; break;
              default:;
                key_pressed=0;
            }
            digitalWrite(KB_LED, LOW);             
          } 
          else {
            SET_KEY_IN(s); // suspect to up
          }
        } else {
          CLR_KEY_IN(s);
        }
      }
    }
    u<<=1;
  }
  
  if(key_pressed && rep++>=REP_COUNT) {
    //Serial.print(key_pressed);
    //if(flags&F_SER_ECHO_ON) term.printc(key_pressed);
    EMIT(key_pressed);
    rep=0;
  }
}
