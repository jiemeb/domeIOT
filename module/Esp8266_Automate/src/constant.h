


enum { RESET_BYTE = 0, SET_BYTE, RESET_BIT,SET_BIT,READ_BYTE, READ_BIT,ANALOG_READ,ANSWER ,SET_TIME,GET_TIME,SET_CHRONO,SET_PROGRAMME,RESET_EEPROM ,SET_ID, SET_TEMP_OFFSET, GET_TEMP, TIME_CALIBRATION };
enum { COMMAND=2,VALUE, VALUE1, VALUE2 };
enum { INDEX=0, HIGH_TIME=0 , LOW_TIME ,ZERO_BYTE , ONE_BYTE };

// format answer

enum { INDEX_ANSWER=0, INDEX_ORDER, INDEX_STATUS, INDEX_VALUE  };

#define TIME_AFTER_SEND	1
#define TIME_BEFORE_SEND	500


#define DEBUG_PRINT(x)    Debug.print(x)
#define DEBUG_PRINT2(x,y)    Debug.print(x,y)
#define DEBUG_PRINTF(x,y)   Debug.printf(x,y)
#define DEBUG_PRINTLN(x)  Debug.println(x)
#define DEBUG_PRINTLN2(x,y)  Debug.println(x,y)
#define DEBUG_WRITE(x,y)    Debug.write(x,y)
