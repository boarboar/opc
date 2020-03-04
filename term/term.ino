
#include <SPI.h>
#include "MspFlash.h"
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

#define LATCH_LOW()  {P2OUT &= ~BIT3;}
#define LATCH_HIGH() {P2OUT |= BIT3;}
#define DATA_LOW()  {P2OUT &= ~BIT4;}
#define DATA_HIGH() {P2OUT |= BIT4;}
#define CLOCK_LOW()  {P2OUT &= ~BIT5;}
#define CLOCK_HIGH() {P2OUT |= BIT5;}

const uint16_t rowPins[ROW_COUNT] = {P2_2, P2_1, P2_0, P1_4}; //can't use P1_5 - spi clk

inline uint8_t readRow(uint8_t row) {
  switch(row) {
    case 0: return P2IN&BIT2 ? HIGH : LOW; break;
    case 1: return P2IN&BIT1 ? HIGH : LOW; break;
    case 2: return P2IN&BIT0 ? HIGH : LOW; break;
    case 3: return P1IN&BIT4 ? HIGH : LOW; break;
    default: return LOW;    
  }
}

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
#define KEY_FLASH_WRITE    0xf6
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
{KEY_ESC, 0, 0, 0, 0, KEY_ESC_L, KEY_ESC_U, KEY_ESC_D, KEY_ESC_R, '\b'},  //  l, u, d, r, bsp
{'\t', '~', 0, 0, '-', '_', '=', '+', '\\', '|'},
{KEY_DEL, 0, ';', ':', '"', '\'', '[', '{', ']', '}'},
{0, 0, 0, 0, ',', '<', '.', '>', '/', '?'}
};

const uint8_t keymap_shift_fn[ROW_COUNT][COL_COUNT] = {
{0, KEY_CTRL_C, KEY_CTRL_D, 0, 0, 0, 0, 0, 0, 0},  
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0,0,KEY_SERIAL_SPEED, KEY_SERIAL_ECHO, 0, KEY_FLASH_WRITE, 0, 0, KEY_CTRL_C, KEY_CTRL_D}
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

#define EMIT(K) { Serial.print((K)); if(flags&F_SER_ECHO_ON) { term.hideCursor(); term.printc((K)); } }  
#define PRINT_ECHO_STATUS() {term.println((flags&F_SER_ECHO_ON) ? "[ECHO ON]" : "[ECHO OFF]");}
#define PRINT_XCTRL_STATUS() {term.println((flags&F_SER_XCTRL_ON) ? "[XONOFF ON]" : "[XONOFF OFF]");}

#define CODE_XOFF  '\x13'
#define CODE_XON   '\x11' 

#define EMIT_XON() {if(flags&F_SER_XCTRL_ON) Serial.print(CODE_XON);}
#define EMIT_XOFF() {if(flags&F_SER_XCTRL_ON) Serial.print(CODE_XOFF);}

#define flash SEGMENT_D

void setup() {                
  // initialize the digital pin as an output.
  digitalWrite(KB_LED, LOW); 
  pinMode(KB_LED, OUTPUT); 
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
 
  for(int row=0; row<ROW_COUNT; row++) pinMode(rowPins[row], INPUT_PULLUP);  
  
  digitalWrite(KB_LED, HIGH);
  flash_read();
  Serial.begin(serial_rates[serial_rate_idx]);  
  EMIT_XOFF();
  term.init();  
  delay(10);
  while(Serial.available()>0) Serial.read(); // clear input
  term.prints("TERM v1.1 "); 
  term.prints(serial_rates_descr[serial_rate_idx]);
  PRINT_ECHO_STATUS();
  //PRINT_XCTRL_STATUS();
  
  digitalWrite(KB_LED, LOW);
  t=millis();
  EMIT_XON();
}

void loop() {
  if(Serial.available()>0) {
    uint16_t cc=0;  
    int16_t b;
    term.hideCursor();
    while(/*Serial.available()>0 && */cc++<640 && ((b=Serial.read())!=-1)) { // limit to 400 chars at once to give kbhandler a chance
      //EMIT_XOFF();
      //byte b = Serial.read();       
      term.printc((char)b);
      //EMIT_XON();
      // if(cc>16) Serial.print(CODE_XOFF); // try this
    }
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
    /*
    digitalWrite(latchPin, HIGH); //Pull latch HIGH to send data
    shiftOut(dataPin, clockPin, MSBFIRST, (~u)>>8); //Send the data HIBYTE
    shiftOut(dataPin, clockPin, MSBFIRST, (~u)&0xFF); //Send the data LOBYTE
    digitalWrite(latchPin, LOW); // Pull latch LOW to stop sending data
    */
    
    LATCH_HIGH();
    rval = (~u)>>8;
    for (s = 0; s < 8; s++)  {
      if( !!(rval & (1 << (7 - s))) ) DATA_HIGH()
      else DATA_LOW();
      CLOCK_HIGH();
      CLOCK_LOW();      
    }
    rval = (~u)&0xFF;
    for (s = 0; s < 8; s++)  {
      if( !!(rval & (1 << (7 - s))) ) DATA_HIGH()
      else DATA_LOW();
      CLOCK_HIGH();
      CLOCK_LOW();      
    }    
    LATCH_LOW();
    
    for(uint8_t row=0; row<ROW_COUNT; row++) { 
      //rval=digitalRead(rowPins[row]); // 16_t ???
      rval = readRow(row);
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
                  ((flags&F_SHIFT_PRES) && (flags&F_FN_PRES)) ? keymap_shift_fn[row][col] :
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
                } else if((uint8_t)key==KEY_FLASH_WRITE) {
                  flash_write();
                  term.println("FWR");
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

const unsigned char flash_magic=0x57;

boolean flash_read() {
  unsigned char p = 0;
  Flash.read(flash+0, &p, 1);
  if(p!=flash_magic) return false;
  Flash.read(flash+1, &p, 1);
  if(p<SERIAL_RATES_N) serial_rate_idx=p;
  Flash.read(flash+2, &p, 1);
  if(p) flags|=F_SER_ECHO_ON;
  else flags&=~F_SER_ECHO_ON;
}

boolean flash_write() {
  unsigned char p;
  Flash.erase(flash);
  p = flash_magic;
  Flash.write(flash+0, &p, 1);
  p = serial_rate_idx;
  Flash.write(flash+1, &p, 1);
  p = flags&F_SER_ECHO_ON ? 1 : 0;
  Flash.write(flash+2, &p, 1);
  return true;
}
