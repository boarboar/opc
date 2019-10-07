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
#define LED RED_LED
/*
#define latchPin P2_3
#define dataPin P2_4
#define clockPin P2_5
*/

#define latchPin P2_2
#define dataPin P2_1
#define clockPin P2_0


const byte COL_COUNT = 4;
const byte ROW_COUNT = 2;
//unsigned char sequence[8] = {B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};

const uint16_t rowPins[ROW_COUNT] = {P2_3, P2_4};

#define STATE_UP  0
#define STATE_TO_DN  1
#define STATE_DN  2
#define STATE_TO_UP  3
uint8_t state[COL_COUNT][ROW_COUNT] = {STATE_UP};

#define SCAN_DELAY 20

void setup() {                
  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT); 

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  for(int row=0; row<ROW_COUNT; row++) pinMode(rowPins[row], INPUT_PULLUP);    
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(SCAN_DELAY);               // wait scan
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);               // wait for a second
  uint16_t u=1;
  for (int col = 0; col < COL_COUNT; col++)   
  {
    uint16_t mask=~u;
    //Serial.print("COL:");
    //Serial.print(col);
    digitalWrite(latchPin, HIGH); //Pull latch HIGH to send data
    shiftOut(dataPin, clockPin, MSBFIRST, mask>>8); //Send the data HIBYTE
    shiftOut(dataPin, clockPin, MSBFIRST, mask&0xFF); //Send the data LOBYTE
    //shiftOut(dataPin, clockPin, MSBFIRST, sequence[col]); //Send the data LOBYTE
    digitalWrite(latchPin, LOW); // Pull latch LOW to stop sending data
    
    //Serial.print("  ROW: ");
    for(int row=0; row<ROW_COUNT; row++) {
      uint16_t rval=digitalRead(rowPins[row]);
      
      if(rval==LOW) { // DN
        switch(state[col][row]) {
          case STATE_UP : 
            state[col][row] = STATE_TO_DN;
            //Serial.print(col);
            //Serial.print(",");
            //Serial.print(row);
            //Serial.println(" = TO_DN");
            break;
          case STATE_TO_DN : 
            state[col][row] = STATE_DN;
            Serial.print(col);
            Serial.print(",");
            Serial.print(row);
            Serial.println(" = DN!!!");
            break;
          case STATE_TO_UP : 
            state[col][row] = STATE_DN;
            //Serial.print(col);
            //Serial.print(",");
            //Serial.print(row);
            //Serial.println(" = BDN");
            break;
          default: break;  
        }
      } else { // UP
        switch(state[col][row]) {
          case STATE_DN : 
            state[col][row] = STATE_TO_UP;
            //Serial.print(col);
            //Serial.print(",");
            //Serial.print(row);
            //Serial.println(" = TO_UP");
            break;
          case STATE_TO_UP : 
            state[col][row] = STATE_UP;
            Serial.print(col);
            Serial.print(",");
            Serial.print(row);
            Serial.println(" = UP!!!");
            break;
          case STATE_TO_DN : 
            state[col][row] = STATE_UP;
            //Serial.print(col);
            //Serial.print(",");
            //Serial.print(row);
            //Serial.println(" = BUP");
            break;
          default: break;  
        }
      }
    }
      //Serial.print(rval);
      //Serial(); 
    //delay(500);
    u<<=1;
  }
}

