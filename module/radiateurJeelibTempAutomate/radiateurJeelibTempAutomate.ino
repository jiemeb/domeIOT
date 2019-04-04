



/// Must define something about Licence 
/// To be define

//#include <SoftwareSerial.h>
//SoftwareSerial Serial(10,9); // RX, TX

#include <JeeLib.h>
#include "automate.h"
#include "internalTemperature.h"
#include "sequence.h"


#include <EEPROM.h>
// static const char mes[] ="Send and Receive";

Automate A;
internalTemperature T;
sequenceur S;
volatile temptx temptxIn;
int lastSecconde;


void setup () {

  Serial.begin(9600);
  //Serial.println(F("9600"));
    A.setup();
    T.setup();
    S.setup();
	
//  Serial.println((fstr_t*) PSTR("Send and Receive"));
 // Serial.println(F("Send and Receive"));
  Serial.println(EEPROM.read(MY_NODE),HEX);
//   rf12_initialize(nodeId, RF12_433MHZ, GroupId ,0x640); // 1600 freq
rf12_initialize(   EEPROM.read(MY_NODE), RF12_433MHZ, 212,0x640); // 1600 freq


#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
  Serial.println((fstr_t*) PSTR("tiny 84 "));
  Serial.println(SIGNATURE,DEC);
#endif

  delay (1000);
  
   Serial.println(F("ROM"));
    for ( short   i=0;i < SIZE_EEPROM; i+=16)
    {
        for ( char j = 0 ; j < 16 ; j++)
           { Serial.print( EEPROM.read(j+i),HEX);
            Serial.print(F(" ")); }
           
            Serial.println();
    }

}

void loop ()
{
unsigned short intermediate ;

 if ((rf12_recvDone()) && rf12_crc == 0 )
  {
        Serial.print(F(" REC "));  
    for (unsigned char  i = 0; i < rf12_len; ++i)
    {
      Serial.print(rf12_data[i],HEX);
      Serial.print(" ");
    }
   
    A.decodeMessage(&rf12_data[0]);
   


    delay(5); 

   Serial.print (F("\r\nJour "));
   intermediate= A.jour;
  Serial.println ((intermediate),DEC);
   Serial.print (F("Heure "));
   intermediate= A.minutes/60;  
   Serial.println ((intermediate),DEC);
   Serial.print (F("Minutes "));
   intermediate= A.minutes%60;
   Serial.println ((intermediate),DEC);
  }

  A.live() ;
 
if ( A.secondes != lastSecconde)
{
lastSecconde = A.secondes;
  	T.live();
    S.live() ;
  if ( A.secondes & 1 )
    A.setBit(0);
  else
    A.resetBit(0);
}

}
