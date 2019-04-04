
#define MAXINT 32767
#define MININT -32767


extern volatile temptx temptxIn;

 

class internalTemperature 
{

public:


volatile short offset;
float coefficient=1;


//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------

short readVcc() ;
short in_c() ;
short  raw() ;

void sprint() ;

 

  void live( );
  void setup ();	
 //void int_sensor_init();								

};		 
