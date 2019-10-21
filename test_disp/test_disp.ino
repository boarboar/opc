#include <SPI.h>

#include "terminal.h"

LCDTerminal t;
  
void setup()
{
  delay(100);
  pinMode(RED_LED,OUTPUT);
  t.init();  
  for(int j=0;j<WS_CHAR_N_Y;j++)
    for(int i=0; i<WS_CHAR_N_X; i++)    
    t.print('a'+j);
}

void loop()
{
   digitalWrite(RED_LED, HIGH);
   delay(500);
   digitalWrite(RED_LED, LOW);
   delay(500);
   t.scroll(); 
}

