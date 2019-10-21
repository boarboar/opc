#include <SPI.h>

#include "terminal.h"

LCDTerminal t;
  
void setup()
{
  delay(100);
  pinMode(RED_LED,OUTPUT);
  t.init();  
  for(int i=0; i<60; i++)    
    t.print('A'+i);
}

void loop()
{
   digitalWrite(RED_LED, HIGH);
   delay(500);
   digitalWrite(RED_LED, LOW);
   delay(500);
   t.scroll(); 
}

