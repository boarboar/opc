#include <SPI.h>

#include "terminal.h"

LCDTerminal t;

int8_t c = 0;
int8_t ic = 0;
char sep[]="--_--\t+++\t---\b\b\b\b\b";
void setup()
{
    Serial.begin(9600); 
    
  pinMode(RED_LED,OUTPUT);
  t.init();  
  delay(100);
  t.println("TERM v1.0");
  
  for(int j=0;j<WS_CHAR_N_Y-2;j++)
    for(int i=0; i<WS_CHAR_N_X; i++)    
    t.print('a'+j);
    
 Serial.println("TEST");   
}

void loop()
{
   digitalWrite(RED_LED, HIGH);
   t.showCursor();
   delay(500);
   t.hideCursor();
   digitalWrite(RED_LED, LOW);
   delay(500);
   
   for(int i=0; i<WS_CHAR_N_X; i++)    
     t.print('0'+c); 
   if(++c>=10) {
     c=0;
     sep[2]='0'+ic;     
     ic++;
     t.print(sep);
     t.println("@@@");
   }  
   
}

