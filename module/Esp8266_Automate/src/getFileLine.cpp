#include "getFileLine.h"


getFileLine::getFileLine () {
currentPointer = 0 ;
file = (File) 0;
minimunChar = 5;
}

int getFileLine::setFile (String  pathFile,unsigned short min )
{
  currentPointer = 0 ;
  file = (File) 0;
  minimunChar = min;
  file = LittleFS.open(pathFile, "r");
  if ( file != 0)
    return (0);
return -1; 
}



getFileLine::~getFileLine ( ) {
currentPointer = 0 ;
if (file != 0)
  file.close();
file = (File) 0;
minimunChar = 5;

}

unsigned short getFileLine::getLine ( char  * line) {
currentPointer = 0 ;

  if (file != (File)NULL)
    {

			while (file.read( (uint8_t* )&line[currentPointer],1) != 0)
 			{
				if (line [currentPointer] != '\n' && line [currentPointer] != '\r') // Get uint8_tacter  until newline ou Carrriage return
	 			{
					currentPointer++;
					continue ;
	  		    }
				if(currentPointer < minimunChar)
        {// Skip line under min uint8_tacter
					continue ;
            }
      break;    //Line is completed
      }
    }
return (currentPointer);

}

 short getFileLine::getPos()
 {
   if (file )
    return (currentPointer) ;
  return (-1 ) ;
}
