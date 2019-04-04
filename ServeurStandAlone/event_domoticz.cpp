#include <Arduino.h>
#include <string.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
// WiFi information

#include <stdio.h>
#include <FS.h>
#include "event_domoticz.h"
#include "getFileLine.h"
// Global variables
WiFiClient client;
HTTPClient http;
char equ_bit[29][8];
char equ_temp[29];

void sendDomoticz(String url)
{
Serial.print("connecting to ");
Serial.println(sendToHost);
Serial.print("Requesting URL: ");
Serial.println(url);
http.begin(sendToHost,sendToPort,url);
int httpCode = http.GET();
	if (httpCode) {
		if (httpCode == 200) {
			String payload = http.getString();
			Serial.println("Domoticz response ");
			Serial.println(payload);
		}
		else
		{
			Serial.print("Domoticz response ");
			Serial.println(httpCode);
	   }
	}
Serial.println("closing connection");
http.end();
}

void event_domoticz_init()
{

int i;int j; int k;
for (  j = 0; j < 29; j++)
 {
 for ( i = 0; i< 8 ; i++ )
	{
		equ_bit[j][i] = 0 ;
	}
		equ_temp[j] = 0 ;
}
  printf("Init event_domoticz");
  File  file = SPIFFS.open("/event_domoticz.ini", "r");

    if (file != NULL)
    {
			i=0;
			j=0 ;
			k = 0;
			uint8_t line[40 ];
			uint8_t   l = 0;
			while (file.read( &line[l],1) != 0)
 			{
				if (line [l] != '\n')
	 			{
					l++;
					continue;
	  		    }
				if(l < 5)
					continue ;
			sscanf((char *)line, "%x %x %d", &j,&i,&k);

		/* J is number of the Card and first card is 2
		Zero of ta is for card to then card 2 is index zero -> j-2
		i indice Bit or 255 if temperature
		k indice domoticz */
			if( j<2 || i <0 || k < 0)
			{
				printf("Erreur File j %x,i %x, k %d \n",j,i,k);
				break;
			}
			if ( i == 0xFF )
			{
				equ_temp[j-2] = k ;
				printf("Sondes temp  %x %x %d \n", j,i,equ_temp[j-2]);
			}
			else
			{
				equ_bit[j-2][i] = k ;
				printf("Sondes  %x %x %d \n", j,i,equ_bit[j-2][i]);
			}


			l = 0;											 // for next line
			memset(line, 0, sizeof line) ;
		}
    file.close();
    }
		else {
		printf("/event_domoticz.ini Not Found");
	}
}


void event_domoticz_event_temp(char card,char value)
{
//	const char event_temp[] ="http://127.0.0.1:8080/json.htm?type=command&param=udevice&idx=%d&nvalue=0&svalue=%d";
	const char event_temp[] =TEMP;
	char mes_post[255] ;

	sprintf(mes_post, event_temp,equ_temp[-2+card],value) ;
  sendDomoticz( mes_post); //send notification Temperatur


}

void event_domoticz_event_bit(char card,char bit,char value)
{

//	const char event_bit[] ="http://127.0.0.1:8080/json.htm?type=command&param=switchlight&idx=%d&switchcmd=%s" ;
	const char event_bit[] =SWITCH ;
	const char event_bit_etat[2][4]= { "Off","On" } ;
	char mes_post[255] ;

	sprintf(mes_post,event_bit,equ_bit[-2+card][bit],event_bit_etat[value & 1]) ;
	sendDomoticz( mes_post); //send notification




}
