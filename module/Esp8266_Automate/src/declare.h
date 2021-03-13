
#define SIGNATURE 204                     // Version release
#define TimeBeforeSend 320
#define NODE_ID_TO_BE_DEFINE 30
#define SEND_TO_ALL 0x1F
#define SIZE_IO 8
#define SIZE_CHRONO 4*8*8
#define SEQUENCE SIZE_CHRONO
#define SEQUENCE_SIZE 240	// 120 Instruction
#define SIZE_EEPROM 512
#define SIZE_RECORD_CHRONO 4
#define CALIBRATION_TIME 1500

#define ZERO_EEPROM 0x7F
#define JOUR 1440
#define MY_NODE SIZE_EEPROM-2              // Node of my card
#define MY_OFFSET MY_NODE-2              // temperature Offset 
#define TEMPERATURE_OFFSET -2530          // (20-273)  *10 .1 Ce;sius   
#define INTEGRAL       7     // define integral of temperaratur must be lower  10

struct  temptx {
  	  short temp;	// Temperature reading
  	  short supplyV;	// Supply voltage
 } ;
