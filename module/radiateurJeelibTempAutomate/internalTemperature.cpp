///
/// Live to get temperature
/// Set Byte / Bit and Reset  are made with true value
/// internalTemperature


#include <EEPROM.h>
#include <JeeLib.h>
#include "declare.h"
#include "internalTemperature.h"


//#define IDEBUG
//#define IDEBUG1

//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------

 short internalTemperature::readVcc() {
 short result;
   // Read 1.1V reference against Vcc
   ADMUX = _BV(MUX5) | _BV(MUX0);
   delay(6); // Wait for Vref to settle
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   return result;
}


void internalTemperature::sprint() {
 #ifdef IDEBUG1
  Serial.print( F("> R:") );
  Serial.print( raw (), DEC );
  Serial.print(F( " C:" ));
  Serial.print( in_c(), DEC );

  Serial.println(F( " # ") );
#endif

}

short internalTemperature::in_c() {
  short raw_temp ;
 while( ( ( raw_temp = raw() ) < 0 ) );  // Wait first conversion
  temptxIn.temp = temptxIn.temp + (((raw_temp+offset)-temptxIn.temp)/INTEGRAL) ;

    return (temptxIn.temp);
}


short internalTemperature::raw() {
  if( ADCSRA & _BV( ADSC ) ) {
    return -1;
  } else {
    short ret = ADCL | ( ADCH << 8 );   // Get the previous conversion result
    ADCSRA |= _BV(ADSC);              // Start new conversion
    return (ret*10);                    // to be on .1 degree Celsius
  }
}



/*--------------------------------------------------------internalTemperature  ---------------------------------------*/
void internalTemperature::live()
{
  setup();
  bitClear(PRR, PRADC); // power up the ADC
  ADCSRA |= bit(ADEN); // enable the ADC
  delay(10); // Allow 10ms for the sensor to be ready

  //analogReference( INTERNAL1V1 );
  // Configure ADMUX

  ADMUX = B00100010;                // Select temperature sensor
  ADMUX &= ~_BV( ADLAR );       // Right-adjust result
  ADMUX |= _BV( REFS1 );                      // Set Ref voltage
  ADMUX &= ~( _BV( REFS0 ) );  // to 1.1V
  // Configure ADCSRA
  ADCSRA &= ~( _BV( ADATE ) |_BV( ADIE ) ); // Disable autotrigger, Disable Interrupt
  ADCSRA |= _BV(ADEN);                      // Enable ADC
  ADCSRA |= _BV(ADSC);          // Start first conversion
  // Seed samples
  short raw_temp;
  while( ( ( raw_temp = raw() ) < 0 ) );  // Wait first conversion

  sprint();
  in_c() ; // Convert temperature to an integer, reversed at receiving end
  temptxIn.supplyV = readVcc(); // Get supply voltage

  #ifdef IDEBUG
  Serial.print( readVcc(), DEC );
  Serial.println(F( " # ") );
  #endif

  ADCSRA &= ~ bit(ADEN); // disable the ADC
  bitSet(PRR, PRADC); // power down the ADC

}

/*-------------------------------------------------------- Setup internalTemperature  ---------------------------------------*/

void  internalTemperature::setup ()
{
offset = EEPROM.read ( MY_OFFSET )<< 8;
offset |= EEPROM.read ( MY_OFFSET+1 );
  #ifdef IDEBUG
    Serial.println((F("Offset Value #")));
    Serial.println(EEPROM.read(MY_OFFSET),DEC);
    Serial.println(EEPROM.read(MY_OFFSET+1),DEC);
    Serial.println(offset,DEC);
     Serial.print((F("#")));
   #endif
}
