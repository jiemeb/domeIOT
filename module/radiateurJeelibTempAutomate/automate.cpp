///
/// Message Decode for IO manipulation
/// Set Byte / Bit and Reset  are made with true value
///


#include <EEPROM.h>
#include <JeeLib.h>

#include "automate.h"
#include "sequence.h"
//#include <SoftwareSerial.h>
//extern SoftwareSerial Serial; // RX, TX

extern class sequenceur S;
//#define DEBUG
//#define DEBUG1
#define DEBUG2
/*--------------------------------------------------------BYTE Manipulation ---------------------------------------*/

char Automate::resetByte(volatile unsigned char  value )
{
  unsigned char i = 0;

  while (  i< SIZE_IO && t_out[i] >= 0 )
  {
    if (value & 1)
      digitalWrite(t_out[i], LOW);
    i++;
    value >>= 1;
  }

  return (0);
}

char Automate::setByte(volatile  unsigned char  value )
{
  int i = 0;

  while (  i< SIZE_IO && t_out[i] >= 0 )
  {
    if (value & 1)
      digitalWrite(t_out[i], HIGH);
    i++;
    value >>= 1;
  }

  return (0);
}

char  Automate::readByte( )
{
  unsigned char value;
  unsigned char i = 0;
  value = 0;
  while (  i< SIZE_IO  )// Circular read Bit and compose Byte
  {
    value >>= 1;
    if ( t_in[i] >= 0 ) // test end of IO
    {
      if (digitalRead(t_in[i])) // test value
        value |= 0x80;     // True
      else
        value &= 0x7F;      // False
    }
    else
    {
      value &= 0x7F;
    }
    i++;

  }

#ifdef DEBUG1
  Serial.print((F("Valeur read_")));
  Serial.println(value,HEX);
#endif     

  return (value);
}

/*--------------------------------------------------------BIT Manipulation ---------------------------------------*/

char Automate::resetBit( volatile  unsigned char  value )
{
if (value & 0x80)
{
markers &= ~(1<<(value & 0x7F));
}
else
{
  if  (t_out[value] < 0)
    return(-1);
  digitalWrite(t_out[value], LOW);
}
  return(0);
}

char Automate::setBit( volatile  unsigned char  value )
{
if (value & 0x80)
{
markers |= (1<<(value & 0x7F));
}
else
{
  if  (t_out[value] < 0)
    return(-1);
  digitalWrite(t_out[value], HIGH);
}
  return(0);
}
char Automate::readBit( volatile unsigned char  value )
{
if (value & 0x80)
{
if (markers & (1<<(value & 0x7F)))
	value = 1 ;
else
	value = 0 ;
}
else
{

  if  (t_in[value] < 0)
    return(-1);
  if ( digitalRead(t_in[value]))
  {
    value=1;
  }
  else
  {
    value=0;
  }
}
#ifdef DEBUG1
  Serial.print((F ("ReadBit")));
  Serial.println(value,HEX);
#endif

  return (value);
}

/*--------------------------------------------------------Time  Manipulation ---------------------------------------*/

char Automate::setTime(volatile unsigned short  value,char valSecondes ) // Jour de 0-6 + Minutes du jour , secondes
{
#ifdef DEBUG1
  Serial.print((F ("SetTimer __")));
  Serial.println(value,HEX);
#endif
  jour = (value >> 12 );


  if ( calibrationTime  && !holdMinutes) // Init TCalibration
    {
  	holdMinutes = value ;
  	holdSecondes = valSecondes;
    }
   else if (calibrationTime )
    {
      if((holdMinutes >> 12) != (value >>12 )) // Not same date retry
        {
        holdMinutes = 0;
        holdSecondes = 0;
        calibrationTime = CALIBRATION_TIME ;
        }
        else
        {
          long reference,real,debut  ;
          debut =((holdMinutes & 0xFFF ) * 60 + holdSecondes);
          reference =((value & 0xFFF) * 60 + valSecondes) - debut;
          real = ((minutes * 60 ) + secondes ) - debut;
          countSecondes = (1000 * real) / reference ;
          if (countSecondes < 850 || countSecondes > 1300)
                      countSecondes = 1000;
#ifdef DEBUG1
 	 Serial.print(F ("New Count__"));
 	 Serial.println(countSecondes,DEC);
#endif
	secondeTimer.poll(0);		// Stop timer
	secondeTimer.poll(countSecondes) ;	//start new now
       }
    }


  minutes = (value & 0xFFF ) ;
  secondes = valSecondes ;

  return(0);
}
/*--------------------------------------------------------Analogique read---------------------------------------------*/
char Automate::readAByte( volatile unsigned char  value )
{
  if  (t_ai[value] < 0)
    return(-1);
  value=analogRead(t_ai[value]);

#ifdef DEBUG1
  Serial.print(F ("analogRead"));
  Serial.println(value,HEX);
#endif

  return (value);
}




/*--------------------------------------------------------Automate Manipulation ---------------------------------------*/
void Automate::live()
{
  if (secondeTimer.poll(countSecondes))
  {
    secondes += 1   ;
    if ( secondes > 59 )
    {
      secondes = 0 ;
      minutes +=1 ;
      newMinutes = 1;
    }
    if ( minutes > (const short )JOUR )
    {
      minutes = 0 ;
      jour +=1 ;
    }
    if ( jour > 6 )
    {
      jour = 0 ;
    }

    if( newMinutes) // Changement de 1 Minutes
    {
      newMinutes  = 0;
#ifdef DEBUG
            Serial.print((F ("J ")));
            Serial.println(jour,HEX);
            Serial.println(minutes,HEX);

#endif
      for (unsigned short i = 0; i < SIZE_CHRONO; i+=SIZE_RECORD_CHRONO)
      {
        unsigned char k;
        k =(( EEPROM.read(i) & 0XF0) >> 4);
        if ((k == (unsigned char) 7 ) || ( k == jour)) // ALL Days
        {

          if ((((unsigned)(EEPROM.read(i) & 0x0F) <<8 )|(unsigned)  EEPROM.read(i+1)) == minutes)
          {
#ifdef DEBUG1
            Serial.print((F ("Trouver")));
            Serial.println(i,DEC);
#endif
            resetByte (EEPROM.read(i+2));
            setByte(EEPROM.read(i+3));
          }
        }
      }
    }
   // ----------------Calibration time ---------------------------//
    if (calibrationTime ) //Let's go
    {
      if ((calibrationTime == CALIBRATION_TIME) || (calibrationTime == 2)) //Get 1498 Secondes
      {
#ifdef DEBUG1
      Serial.println((F ("Demande Heure"))); // get time
#endif
      char demandeHeure[]={GET_TIME,0};
delay(200+(EEPROM.read(MY_NODE)*5));


      rf12_sendNow (SEND_TO_ALL, demandeHeure, 2);
      //rf12_sendStart (SEND_TO_ALL, demandeHeure, 2);
      rf12_sendWait(0); // No powerdown
//delay(10);

      }
      calibrationTime -= 1;    //decrement Calibration

    }


  }


}



/*--------------------------------------------------------Message Manipulation ---------------------------------------*/


void  Automate::decodeMessage(volatile unsigned char   *message )
{
  unsigned char mes[7] ;
    mes[INDEX_ORDER]=message [COMMAND];
    mes[INDEX_VALUE]= 0 ;
    mes[INDEX_VALUE+1]= 0 ;
  switch (message [COMMAND])  {
  case RESET_BYTE:
    mes[INDEX_STATUS] = resetByte (message[VALUE]);
#ifdef DEBUG
    Serial.println((F("Reset Byte")));
#endif
    break;

  case SET_BYTE:
    mes[INDEX_STATUS] = setByte (message[VALUE]);
#ifdef DEBUG
    Serial.println((F("Set Byte")));
#endif
    break;
  case RESET_BIT:
    mes[INDEX_STATUS] = resetBit(message[VALUE]);
#ifdef DEBUG
    Serial.println((F("Reset Bit")));
#endif
    break;

  case SET_BIT:
    mes[INDEX_STATUS] = setBit(message[VALUE]);
#ifdef DEBUG
    Serial.println((F ("Set Bit")));
#endif
    break;
  case READ_BIT:
   mes[INDEX_STATUS] = message[VALUE];
    mes[INDEX_STATUS+1] = readBit(message[VALUE]);
#ifdef DEBUG
    Serial.println((F ("Read Bit")));
#endif
    break ;
  case READ_BYTE:
    mes[INDEX_STATUS] = readByte();
#ifdef DEBUG
    Serial.println((F ("Read Byte")));
#endif
    break;
  case ANALOG_READ:
    mes[INDEX_STATUS] = readAByte(message[VALUE]);
#ifdef DEBUG
    Serial.println((F("Read analog")));
#endif
    break ;

  case TIME_CALIBRATION:
    // Special
        S.setup();
        holdMinutes = 0;
        holdSecondes = 0;
        calibrationTime = CALIBRATION_TIME -1;

  case SET_TIME:
    mes[INDEX_STATUS] = setTime(((message[VALUE]<< 8 )| message[VALUE1]),message[VALUE2]);
    break;

  case SET_CHRONO:
    if ( setChrono (&message[VALUE]))
    {
#ifdef DEBUG
      Serial.println(F ("Error eeprom "));
#endif
      mes[INDEX_STATUS] =  -1;
    }
    else
      mes[INDEX_STATUS] =  0;
    break;

  case RESET_EEPROM:
    mes[INDEX_STATUS] =  resetEeprom ();
    break;

   case GET_TIME: // Do Nothing it's a another answer
   case ANSWER: // Do Nothing it's a another answer
    mes[INDEX_STATUS] = -1;
    return ;




   case SET_ID:
   #ifdef DEBUG
    Serial.println((F("Set ID")));
   #endif
    EEPROM.write(MY_NODE, message[VALUE]);
    mes[INDEX_STATUS] = 0;
    break;

  /* GET AN SET TEMP Could be On  Other Part */

    case SET_TEMP_OFFSET:
   #ifdef DEBUG
    Serial.println((F("Set offset temp")));
    Serial.print(message[VALUE],DEC);
    Serial.print(message[VALUE1],DEC);
   #endif
   short offset ;
   short decallage ;
   decallage = (message[VALUE]<< 8 | message[VALUE1])*10 ;   // Offset in .1 Celsius
   offset = (EEPROM.read(MY_OFFSET)<< 8 ) | EEPROM.read(MY_OFFSET+1) ;

   offset += decallage;

    EEPROM.write(MY_OFFSET, offset >>8);
    EEPROM.write(MY_OFFSET+1, offset & 0xFF);
    mes[INDEX_STATUS] = 0;
    break;

    case GET_TEMP:
   #ifdef DEBUG
    Serial.println((F("Get TEMP")));
   #endif
    mes[INDEX_STATUS]= (char) (temptxIn.temp / 10 ); // Convert in celsius
    mes[INDEX_VALUE]= (char) ((temptxIn.supplyV)>> 8 ) ;
    mes[INDEX_VALUE+1]= (char) (temptxIn.supplyV) ;
    break;




   default:
     mes[INDEX_STATUS] =  -1;          // Should be error
    #ifdef DEBUG
      Serial.println(F ("Er Order"));
      for(int  i = 0 ; i < 4; i++ )
            Serial.print (*message++,HEX);
    #endif
    break;


  }
  mes[INDEX_ANSWER] = ANSWER ;
  mes[INDEX_VALUE+2]= minutes / 60 ;
  mes[INDEX_VALUE+3] = minutes % 60 ;


delay(TimeBeforeSend);

      rf12_sendNow (SEND_TO_ALL, mes, 7);
      rf12_sendWait(0); // No powerdown

#ifdef DEBUG2
      //  Serial.println(mes[INDEX_STATUS],HEX);
        Serial.print(F (" S "));
      for(int  i = 0 ; i < 7; i++ )
      {
            Serial.print (mes[i],HEX);
            Serial.print (F(" "));
      }
#endif


}

/*--------------------------------------------------------HORO DATE Manipulation ---------------------------------------*/
char  Automate::setChrono (volatile unsigned char  *message)
{

  unsigned  short  indexChrono ;
  indexChrono = message[INDEX] * SIZE_RECORD_CHRONO ;
  if (indexChrono >= (SIZE_CHRONO+SEQUENCE_SIZE))
    return  (-1);
#ifdef DEBUG1
  Serial.print(F ( "Write Chrono " ));
  Serial.println( indexChrono,HEX );
#endif
  for (unsigned short i = 0; i < SIZE_RECORD_CHRONO ; i++)
  {

    EEPROM.write(i+indexChrono, message[i+1]);
#ifdef DEBUG1
    Serial.println(message[i+1],HEX );
#endif
  }
  return (0);

}

char Automate::resetEeprom ()
{
  EEPROM.write(SIZE_EEPROM -1,0);
  setup();
  return(0);
}

/*-------------------------------------------------------- Setup Automate  ---------------------------------------*/

void  Automate::setup ()
{
  unsigned char i = 0;
  jour = 0 ;                // Jour Lundi = 1 ;
  minutes = 0;
  secondes = 0;
  holdMinutes = 0;
  holdSecondes = 0;
  newMinutes = 0;
  calibrationTime = CALIBRATION_TIME ;
  countSecondes = 1000;
  markers = 0 ;
  // Init EEPROM

  if ( SIGNATURE != EEPROM.read(SIZE_EEPROM-1))
  {
    for (unsigned short  i = 0; i < SIZE_EEPROM; i++)
                                       EEPROM.write(i, ZERO_EEPROM);

// Offset of temperatur
     short tempOffset = TEMPERATURE_OFFSET ;
    EEPROM.write ( MY_OFFSET,  tempOffset >> 8);
    EEPROM.write ( MY_OFFSET+1,tempOffset & 0xFF);
// Node Id
    EEPROM.write(MY_NODE, NODE_ID_TO_BE_DEFINE);
//Signatur
    EEPROM.write(SIZE_EEPROM-1,SIGNATURE);


#ifdef DEBUG
    Serial.println(F ("Initialisation EEPROM"));
    Serial.print ( F("Valeur Offset"));
    Serial.println(tempOffset,DEC);
#endif
  }
    while (  i< SIZE_IO && t_in[i] >= 0 )
  {
   if ( t_out [i] != t_in [i])
           pinMode(t_in[i],INPUT);
#ifdef DEBUG1
    Serial.println(t_in[i],HEX);
#endif
    i++;
  }
i = 0;
  while (  i< SIZE_IO && t_out[i] >= 0 )
  {
    pinMode(t_out[i], OUTPUT);
#ifdef DEBUG1
    Serial.println(t_out[i],HEX);
#endif
    i++;
  }
}
