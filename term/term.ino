
#include <SPI.h>
#include "terminal.h"

LCDTerminal term;

#define KB_LED RED_LED

const byte COL_COUNT = 10;
const byte ROW_COUNT = 4;

#define latchPin P2_3
#define dataPin P2_4
#define clockPin P2_5

const uint16_t rowPins[ROW_COUNT] = {P2_2, P2_1, P2_0, P1_4}; //can't use P1_5 - spi clk

#define STATE_UP    0
#define STATE_TO_DN 1
#define STATE_DN    2
#define STATE_TO_UP 3
uint8_t state[COL_COUNT][ROW_COUNT] __attribute__((packed)) = {STATE_UP};

/*
uint8_t keymap[ROW_COUNT][COL_COUNT] = {
{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
{'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
{'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\r'},
{KEY_SHIFT, KEY_FN, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ' '}
};
*/

#define KEY_SHIFT   0xff
#define KEY_FN      0xfe
#define KEY_ESC_LAST   0xfd
#define KEY_ESC_R   0xfd  // escape seq
#define KEY_ESC_D   0xfc  // escape seq
#define KEY_ESC_U   0xfb  // escape seq
#define KEY_ESC_L   0xfa  // escape seq
#define KEY_ESC_FIRST   0xfa
#define KEY_CTRL_C  0x3
#define KEY_CTRL_D  0x4
#define KEY_ESC     0x1b

uint8_t keymap[ROW_COUNT][COL_COUNT] = {  // for SHFT test
{KEY_SHIFT, '2', '3', '4', '5', '6', '7', '8', '9', '0'},
{KEY_FN, 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
{'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\r'},
{'?', '?', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ' '}
};

uint8_t keymap_shift[ROW_COUNT][COL_COUNT] = 
{
{'!','@','#','$','%','^','&','*','(',')'},
{'Q','W','E','R','T','Y','U','I','O','P'},
{'A','S','D','F','G','H','J','K','L','\n'},
{0, 0, 'Z','X','C','V','B','N','M',' '}
};

uint8_t keymap_fn[ROW_COUNT][COL_COUNT] = {
{KEY_ESC,  KEY_CTRL_C, KEY_CTRL_D, 0, 0, KEY_ESC_L, KEY_ESC_U, KEY_ESC_D, KEY_ESC_R, '\b'},  //  l, u, d, r, bsp
{'\t', '~', 0, 0, '-', '_', '=', '+', '\\', '|'},
{0, 0, ';', ':', '"', '\'', '[', '{', ']', '}'},
{0,0,0,0,',','<', '.', '>', '/', '?'}
};


#define F_SHIFT_PRES  0x01
#define F_FN_PRES     0x02

uint8_t cursor_cnt=0;
uint8_t flags=0;
char key_pressed=0;
uint8_t rep = 0;
uint32_t t;

#define SCAN_DELAY 20
#define REP_COUNT  10
#define CURSOR_COUNT  10

void setup() {                
  // initialize the digital pin as an output.
  digitalWrite(KB_LED, LOW); 
  pinMode(KB_LED, OUTPUT); 
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  
  //pinMode(reqPin, INPUT_PULLUP);
  //pinMode(clockReqPin, OUTPUT);
  //pinMode(dataReqPin, OUTPUT);

  for(int row=0; row<ROW_COUNT; row++) pinMode(rowPins[row], INPUT_PULLUP);  
  
  //attachInterrupt(reqPin, req_irq, FALLING);
  
  //digitalWrite(REQ_LED, HIGH);
  //delay(100);
  //digitalWrite(REQ_LED, LOW);
  digitalWrite(KB_LED, HIGH);
  Serial.begin(9600); 
  
  term.init();  
  delay(100);
  term.println("TERM v1.0");
  digitalWrite(KB_LED, LOW);

  t=millis();
}

void loop() {
  if(millis()<t) {
    // wraparound
    t=millis();
    return;
  }
  if(millis()>=t+SCAN_DELAY) {
    key_loop();
    t=millis();
    if(++cursor_cnt>=CURSOR_COUNT) {
      cursor_cnt=0;
      term.cursorBlink();
    }
  }
}

void key_loop() {
  uint16_t u=1;
  uint16_t mask;
  for (uint8_t col = 0; col < COL_COUNT; col++)   
  {
    mask=~u;
    digitalWrite(latchPin, HIGH); //Pull latch HIGH to send data
    shiftOut(dataPin, clockPin, MSBFIRST, mask>>8); //Send the data HIBYTE
    shiftOut(dataPin, clockPin, MSBFIRST, mask&0xFF); //Send the data LOBYTE
    digitalWrite(latchPin, LOW); // Pull latch LOW to stop sending data
    
    for(uint8_t row=0; row<ROW_COUNT; row++) {
      uint16_t rval=digitalRead(rowPins[row]);
      
      if(rval==LOW) { // DN
        switch(state[col][row]) {
          case STATE_UP : 
            state[col][row] = STATE_TO_DN;
            //Serial.print(col);Serial.print(",");Serial.print(row);Serial.println(" = TO_DN");
            break;
          case STATE_TO_DN : 
            state[col][row] = STATE_DN;
            switch(keymap[row][col]) {              
              case KEY_SHIFT: flags|=F_SHIFT_PRES; break;
              case KEY_FN: flags|=F_FN_PRES; break;
              default:
                char key = 
                  flags&F_SHIFT_PRES ? keymap_shift[row][col] :
                  flags&F_FN_PRES ? keymap_fn[row][col] :
                  keymap[row][col];
                if(key) {
                  key_pressed = key;
                  rep=0;
                  if(key <= KEY_ESC_LAST && key >= KEY_ESC_FIRST) {
                    Serial.print('\x1b'); //ESC
                    term.print('\x1b');
                    Serial.print('[');
                    term.print('[');
                    switch(key) {
                      case (char)KEY_ESC_L:
                        key='D';
                        break;
                      case (char)KEY_ESC_R:
                        key='C';
                        break;
                      case (char)KEY_ESC_U:
                        key='A';
                        break;
                      case (char)KEY_ESC_D:
                        key='B';
                        break;  
                      default:;                      
                    }
                  }
                  Serial.print(key);
                  term.print(key);
                  
                }
              /*  
                Serial.print(col);Serial.print(",");Serial.print(row);Serial.print(" = DN (");
                Serial.print((char)keymap[row][col]);
                if(shift_pressed) Serial.print(" SHIFT ");
                if(fn_pressed) Serial.print(" FN ");
                Serial.println(")");
                */
            }
            digitalWrite(KB_LED, HIGH); 
            break;
          case STATE_TO_UP : 
            state[col][row] = STATE_DN;
            //Serial.print(col);Serial.print(",");Serial.print(row);Serial.println(" = BDN");
            break;
          default: break;  
        }
      } else { // UP
        switch(state[col][row]) {
          case STATE_DN : 
            state[col][row] = STATE_TO_UP;
            //Serial.print(col);Serial.print(",");Serial.print(row);Serial.println(" = TO_UP");
            break;
          case STATE_TO_UP : 
            state[col][row] = STATE_UP;
            switch(keymap[row][col]) {
              case KEY_SHIFT: flags&=~F_SHIFT_PRES; break;
              case KEY_FN: flags&=~F_FN_PRES; break;
              default:;
                key_pressed=0;
                //Serial.print(col);Serial.print(",");Serial.print(row);Serial.print(" = UP (");
                //Serial.print((char)keymap[row][col]);
                //Serial.println(")");
            }
            digitalWrite(KB_LED, LOW); 
            break;
          case STATE_TO_DN : 
            state[col][row] = STATE_UP;
            //Serial.print(col);Serial.print(",");Serial.print(row);Serial.println(" = BUP");
            break;
          default: break;  
        }
      }
    }
    u<<=1;
  }
  
  if(key_pressed && rep++>=REP_COUNT) {
    Serial.print(key_pressed);
    rep=0;
  }
}

