#include <Arduino.h>
#include <string.h>
#include "SPIFFS.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <RemoteDebug.h>
#include <ESPmDNS.h>
// WiFi information

#include <stdio.h>
#include <FS.h>
#include "event_domoticz.h"
#include "getFileLine.h"
#include "constant.h"
// Global variables
extern RemoteDebug Debug;
extern WiFiClient client;
HTTPClient http;
#define nombre_sonde 30
#define nombre_bitBySonde 8
char equ_bit[nombre_sonde][nombre_bitBySonde];
char equ_temp[nombre_sonde];

void sendDomoticz(String url)
{

     struct ip4_addr addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(sendToHost, 2000,  &addr);
    if(err){
        if(err == ESP_ERR_NOT_FOUND){
            DEBUG_PORT.printf("Host was not found!");
		}
		else{DEBUG_PORT.printf("Query Failed");}
	}



#ifdef DEBUG
DEBUG_PORT.print("connecting to ");
DEBUG_PORT.println(sendToHost);
DEBUG_PORT.print("Requesting URL: ");
#endif
DEBUG_PORT.println(url);
char  server_add [18] ;
 sprintf(server_add,IPSTR, IP2STR(&addr)); 

http.begin(server_add,sendToPort,url);
int httpCode = http.GET();
	if (httpCode) {
		if (httpCode == 200) {
			String payload = http.getString();
			DEBUG_PORT.println("Domoticz response ");
			DEBUG_PORT.println(payload);
		}
		else
		{
			DEBUG_PORT.print("Domoticz response ");
			DEBUG_PORT.println(httpCode);
	   }
	}
DEBUG_PORT.println("closing connection");
http.end();
}

void event_domoticz_init()
{

int i;int j; int k;
for (  j = 0; j < nombre_sonde; j++)
 {
 for ( i = 0; i < nombre_bitBySonde ; i++ )
	{
		equ_bit[j][i] = 0 ;
	}
		equ_temp[j] = 0 ;
}
  DEBUG_PORT.printf("Init event_domoticz\n");
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
			sscanf((char *)line, "%x %d %d", &j,&i,&k);

		/* J is number of the Card and first card is 2
		Zero of ta is for card to then card 2 is index zero -> j-2
		i indice Bit or 255 if temperature
		k indice domoticz */

//			DEBUG_PORT.printf("lecture j %x, i %x ,k %x \n",j,i ,k);

			if( j<2 || j > nombre_sonde+1 || i <0 || k < 0)
			{
				DEBUG_PORT.printf("Erreur File j %x,i %x, k %d \n",j,i,k);
				break;
			}
			if ( i == 0xFF )
			{
				equ_temp[j-2] = k ;
				DEBUG_PORT.printf("Sondes temp  %x %x %d \n", j,i,equ_temp[j-2]);
			}
			else if (i < nombre_bitBySonde)
			{
				equ_bit[j-2][i] = k ;
				DEBUG_PORT.printf("Sondes  %x %x %d \n", j,i,equ_bit[j-2][i]);
			}


			l = 0;											 // for next line

		
			memset(line, 0, sizeof line) ;
			
		}
    file.close();
    }
		else {
		DEBUG_PORT.printf("/event_domoticz.ini Not Found");
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
