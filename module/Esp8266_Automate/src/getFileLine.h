#include <Arduino.h>
#include <string.h>

#include <stdio.h>
#include <LittleFS.h>

class getFileLine  {

private:
File file ;
unsigned short currentPointer;
unsigned short minimunChar;


public:
getFileLine() ;
int setFile (String  ,unsigned short ) ;
~getFileLine();
unsigned short getLine(char  *  ) ;
short getPos() ;  // Get Position -1 if no file
};
