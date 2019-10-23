#include <SPI.h>

#include "terminal.h"

LCDTerminal t;

int8_t c = 0;
int8_t ic = 0;
char *sep="--%--";
void setup()
{
  pinMode(RED_LED,OUTPUT);
  t.init();  
  delay(100);
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
   for(int i=0; i<WS_CHAR_N_X; i++)    
     t.print('0'+c); 
   if(++c>=10) {
     c=0;
     sep[2]='0'+ic;     
     ic++;
     t.println(sep);
   }  
}

