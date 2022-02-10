#include "declare.h"
#include "constant.h"
#include <Arduino.h>

extern volatile temptx temptxIn;
extern uint8 cardID;


 const char  t_in [SIZE_IO]=  {4,5,12,14, 0xFF,0xFF,0xFF,0xFF };          // output could be read.
//  7,8,0,9,10,0xFF,0xFF,0xFF };          // output could be read.

static const char   t_out [SIZE_IO]= {
 4,5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
//    7,8,0,0xFF,0xFF,0xFF,0xFF,0xFF };
static const char   t_ai [SIZE_IO]= {     // Futur use
  0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };          // Futur use

class Automate
{

public:

  //translation des entrées Physique a théoriques. a renseigner,
  // FF etant non asigné sinon puissancede 2 du bit
  // ce tableau pourrait etre en eeprom.


  volatile unsigned char myNode;              // Node of my card
  volatile unsigned char jour;                // Jour Lundi = 1 ;
  volatile unsigned char secondes ;
  volatile unsigned char  newMinutes;
  volatile unsigned char  holdSecondes ;

 volatile unsigned short markers;             // 16 Bit markers

  volatile unsigned short minutes;
  volatile unsigned short holdMinutes;
  volatile unsigned short calibrationTime;
  volatile unsigned short countSecondes ;     //time to 1 seconde
// struct chrono  { char high_time , char low_time , char zero_byte , char one_byte } ; // Structure
  void decodeMessage(volatile unsigned char   *message ,unsigned int length,String file );
  char resetByte(volatile unsigned char  value );
  char setByte(volatile unsigned char  value );
  char resetBit(volatile unsigned char  value );
  char setBit(volatile unsigned char  value );
  char readByte( );
  char readAByte(volatile unsigned char  value );
  char readBit(volatile unsigned char  value );
  char setTime(volatile unsigned short value, char valSecondes );
  char setChrono (volatile unsigned char  *message);
  char resetEeprom ();
  void live( );
  void setup ();
  void send (volatile unsigned char state ,volatile unsigned char data);

};
