/*
  Blink
  The basic Energia example.
  Turns on an LED on for one second, then off for one second, repeatedly.
  Change the LED define to blink other LEDs.
  
  Hardware Required:
  * LaunchPad with an LED
  
  This example code is in the public domain.
*/

// most launchpads have a red LED
#define KB_LED RED_LED
//#define REQ_LED GREEN_LED

//unsigned char sequence[8] = {B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};

const byte COL_COUNT = 10;
const byte ROW_COUNT = 4;


#define latchPin P2_3
#define dataPin P2_4
#define clockPin P2_5

const uint16_t rowPins[ROW_COUNT] = {P2_2, P2_1, P2_0, P1_5};

/*
#define latchPin  P2_2
#define dataPin   P2_1
#define clockPin  P2_0

const uint16_t rowPins[ROW_COUNT] = {P2_3, P2_4, P2_5, P1_5};
*/

#define STATE_UP    0
#define STATE_TO_DN 1
#define STATE_DN    2
#define STATE_TO_UP 3
uint8_t state[COL_COUNT][ROW_COUNT] = {STATE_UP};

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
{127, '0', '\d', 0, 0, 27, 24, 25, 26, '\b'},  //  l, u, d, r, bsp
{'\t', '~', 0, 0, '-', '_', '=', '+', '\\', '|'},
{0, 0, ';', ':', '"', '\'', '[', '{', ']', '}'},
{0,0,0,0,',','<', '.', '>', '/', '?'}
};

uint8_t shift_pressed=0;
uint8_t fn_pressed=0;
char key_pressed=0;
uint16_t rep = 0;

//volatile uint8_t req=0;

#define SCAN_DELAY 20
#define REP_COUNT  25

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
  delay(100);
  digitalWrite(KB_LED, LOW);

  Serial.begin(9600);
}

void loop() {
  delay(SCAN_DELAY);               // wait scan
  uint16_t u=1;
  for (int col = 0; col < COL_COUNT; col++)   
  {
    uint16_t mask=~u;

    digitalWrite(latchPin, HIGH); //Pull latch HIGH to send data
    shiftOut(dataPin, clockPin, MSBFIRST, mask>>8); //Send the data HIBYTE
    shiftOut(dataPin, clockPin, MSBFIRST, mask&0xFF); //Send the data LOBYTE
    digitalWrite(latchPin, LOW); // Pull latch LOW to stop sending data
    
    for(int row=0; row<ROW_COUNT; row++) {
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
              case KEY_SHIFT: shift_pressed=1; break;
              case KEY_FN: fn_pressed=1; break;
              default:
                char key = 
                  shift_pressed ? keymap_shift[row][col] :
                  fn_pressed ? keymap_fn[row][col] :
                  keymap[row][col];
                if(key) {
                  key_pressed = key;
                  rep=0;
                  Serial.print(key);
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
              case KEY_SHIFT: shift_pressed=0; break;
              case KEY_FN: fn_pressed=0; break;
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
/*  
  if(req) {
    // pop
    // snd
    req=0;
    Serial.println("REQ");
    //digitalWrite(REQ_LED, HIGH);
    //delay(100);
    //digitalWrite(REQ_LED, LOW);
  }
  */
}

/*
void req_irq() {
  req=1;
}
*/
