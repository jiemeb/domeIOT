#include <Arduino.h>
#include <string.h>
#include <time.h>
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <stdint.h>

#include "constant.h"
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RemoteDebug.h>
//extern NTPClient timeClient ;
extern EasyNTPClient timeClient;

//NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 300000);

#include "event_domoticz.h"
#include "getFileLine.h"





#define ANTI_BOUCLE_LEN 20
#define ANTI_BOUCLE_TIME 10




#define  RFM12B_JEE_HDR_DST_BIT 0x40
int jee_id_all = 0x1F; // 31 it's promiscous

struct {uint8_t card ;uint8_t bit; uint8_t etat;time_t time;} buffeur_antiboucle[ANTI_BOUCLE_LEN];
int ibuffeur_antiboucle = 0 ;
extern RFM69 radio ;
extern RemoteDebug Debug;

// Get information from Internet

void  transformOrder( volatile uint8_t *ibuf,int len,String file)
{
 char tempo[100];
int   i;
//int val;
//   uint8_t*  c;
//   fd_set fds;
   time_t tt;
   struct  tm *s_time;
  getFileLine gline ; // Min read 6

//setlogmask (LOG_UPTO (LOG_NOTICE));

//openlog ("RFM12_SERVER", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);




  //event_domoticz_init();	// prepare event for domoticz
				// Start process reading on RFMB12


  //----------------------- lecture Fifo ------------------------------------//
	//		0		1		2	3	4	5
	// Ibuf = lenth of tansmit,Destination-Byte,LOrder , Value , Value 2,Value 3
  //	uint8_t  ibuf[RF12_MAX_SLEN+1], obuf[RF12_MAX_RLEN+1];
	//int status;
  //  	 len = read(fd_rfm12_in, ibuf, 4);

 	if (len > 0) {
		if (ibuf[1] < 31)
		ibuf[1]|=  RFM12B_JEE_HDR_DST_BIT ; // Jeednode id destination should be RFM12B_JEE_HDR_DST_BIT & 0x1F to set destination node
	else
		ibuf[1]= jee_id_all;
							// over it's node id from sender
	int noBoucle = 1 ;			// for antiboucle
	ibuf[0] = 0 ; 				//Lentght of transmit
  switch (ibuf[2]) {
		case '0' :
		    break ;
		case SET_TIME :
  		//	time (&tt);
    //    timeClient.update();
        tt= timeClient.getUnixTime();
			  s_time = localtime(&tt);
    	   ibuf[1] = jee_id_all;
			   ibuf[2] = SET_TIME;
  			 ibuf[3] = (s_time->tm_wday<< 4 )|( ((s_time->tm_hour * 60)+ s_time->tm_min)>> 8)  ;
			   ibuf[4] = ((s_time->tm_hour * 60)+ s_time->tm_min) & 0xFF;
			   ibuf[5] = s_time->tm_sec ;
			for (i=0; i<6; i++)
        {

          sprintf(tempo," [%d] %X ",i, ibuf[i]);
          DEBUG_PRINT(tempo);
			  }
      DEBUG_PRINT(" \n");
            delay(TIME_BEFORE_SEND);
      radio.send(jee_id_all,(const void*)&ibuf[2],4,0);
//			status =   write(rfm12_fd, ibuf, 6);
//			DEBUG_PRINT("W %d\n",status);
   	//	  usleep (TIME_AFTER_SEND);
		break ;
		case GET_TEMP :
		case RESET_EEPROM:
		case READ_BYTE:
		case RESET_BYTE :	//0

		  sprintf(tempo,"pret a emettre %d --->",len);
      DEBUG_PRINT(tempo);
		    for (i=0; i<len; i++)
      {
    		  sprintf(tempo," [%d] %X ",i, ibuf[i]);
    		  DEBUG_PRINT(tempo);
      }
      DEBUG_PRINTLN(" ");
            delay(TIME_BEFORE_SEND);
      radio.send(ibuf[1],(const void*)&ibuf[2],len-2,0);
      delay(TIME_AFTER_SEND);
      //		status =write(rfm12_fd, ibuf, len-1);
	//	  DEBUG_PRINT("W %d\n",status);
	   //	usleep(TIME_AFTER_SEND);
  	 break;

		case SET_BIT :		//3
		case RESET_BIT :	//2
				// Here I would like to test if anti loopback is on
				// Try to find order in Buf with time min 4 second

			//time ( &tt );
//      timeClient.update();
      tt= timeClient.getUnixTime();
			tt -= ANTI_BOUCLE_TIME ; // Could event went 4 second before
			volatile int k;
      	uint8_t card ;
        card = ibuf[1] & ~RFM12B_JEE_HDR_DST_BIT;
			for ( k=0; k < ANTI_BOUCLE_LEN;k++)
			{
	//printf("Buffeur %d bit %d e %d \n ",k,buffeur_antiboucle[k].bit,buffeur_antiboucle[k].etat);
        if (buffeur_antiboucle[k].time > tt)
          {
			         if (buffeur_antiboucle[k].card == card)
                {
				           if (buffeur_antiboucle[k].bit == ibuf[3])
				               {

//				printf("Buffeur bit %d etat %d \n ",buffeur_antiboucle[k].bit,buffeur_antiboucle[k].etat);

				            if (((buffeur_antiboucle[k].etat == 1) && (  ibuf[2] == SET_BIT )) || ((buffeur_antiboucle[k].etat == 0) && ( ibuf[2] == RESET_BIT )))
						              { // Bingo do nothing
						                      sprintf(tempo,"antiboucle %x \n\r",ibuf[1]);
                                  DEBUG_PRINT(tempo);
						                      noBoucle = 0;
						                      break ;
						               }

                }
              }
 				}
			}
		case SET_BYTE :
		case READ_BIT:
		case ANALOG_READ:
		case SET_ID : 		//1

		//len +=1;
    if (len >= 5 )
        len = 5 ;
		//read(fd_rfm12_in, &ibuf[4], 1);
    sprintf(tempo,"pret a emettre %d --->",len);
    DEBUG_PRINT(tempo);
      for (i=0; i<len; i++)
    {
        sprintf(tempo," [%d] %X ",i, ibuf[i]);
        DEBUG_PRINT(tempo);
    }
    DEBUG_PRINTLN(" ");
		if (noBoucle ) // Antiboucle ON
		{
            delay(TIME_BEFORE_SEND);
  radio.send(ibuf[1],(const void*)&ibuf[2],len-2,0);
//		status = write(rfm12_fd, ibuf, len-1);
		//printf("W %d\n",status);

		}
  		break;


		case SET_TEMP_OFFSET : //E 14
    if (len >= 6 )
        len = 6 ;
		//len +=2;
		//read(fd_rfm12_in, &ibuf[4], 2);
    sprintf(tempo,"pret a emettre %d --->",len);
    DEBUG_PRINT(tempo);
      for (i=0; i<len; i++)
    {
        sprintf(tempo," [%d] %X ",i, ibuf[i]);
        DEBUG_PRINT(tempo);
    }
          delay(TIME_BEFORE_SEND);
  radio.send(ibuf[1],(const void*)&ibuf[2],len-2,0);
	//	status=write(rfm12_fd, ibuf, len-1);
	//	printf("W %d\n",status);
		//usleep(TIME_AFTER_SEND);
delay(TIME_AFTER_SEND);
		break;

    case SET_CHRONO :DEBUG_PRINT("Set chrono ");

		int index,jour,heure,minutes,reset,set,c;

     gline.setFile (file, 6); // Min read 6

 		if( ( gline.getPos()) < 0)
     {
       DEBUG_PRINT("pas de fichier "+file);
       break;
     }
		c=0;
	 	while ((gline.getLine (tempo)))
     		{
      		if( 6 != sscanf(tempo, "%d %d %d %d %x %x\n",&index, &jour,&heure,&minutes,&reset,&set))
		          break;
      		sprintf(tempo,"index %d Jour %d heure %d minute %d reset %X set %X\n",index, jour,heure,minutes,reset,set);
          DEBUG_PRINT(tempo);
       ibuf[0] = 0 ;
  		 ibuf[2] = SET_CHRONO;
 			 ibuf[3] = (uint8_t) index;
			 ibuf[4] = (uint8_t) jour<< 4;
  		 ibuf[4] |= ((heure*60)+minutes)>> 8  ;
			 ibuf[5] = ((heure*60)+minutes) & 0xFF;
			 ibuf[6] = reset ;
			 ibuf[7] = set ;


			for (i=0; i<8; i++)
            {
                sprintf(tempo," [%d] %X ",i, ibuf[i]);
                DEBUG_PRINT(tempo);
            }
			DEBUG_PRINT(" \n");
      delay(TIME_BEFORE_SEND);
      radio.send(ibuf[1],(const void*)&ibuf[2],6,0);

//		  status=write(rfm12_fd, ibuf, 8);
		 // printf("W %d\n",status);
     delay(TIME_AFTER_SEND);
   		//  usleep (TIME_AFTER_SEND);
		     c++;
    		 }

		      sprintf(tempo,"nombre de chrono %d \n",c);
          DEBUG_PRINT(tempo);

    break ;



		case TIME_CALIBRATION :
//    timeClient.update();
    tt= timeClient.getUnixTime();
  	//		time (&tt);
  //      s_time =   timeClient.G;
	//		s_time = localtime(&tt);
			   ibuf[1] = jee_id_all;
			   ibuf[0] = 0 ;
			   ibuf[2] = TIME_CALIBRATION;
  			 ibuf[3] = (s_time->tm_wday<< 4 )|( ((s_time->tm_hour * 60)+ s_time->tm_min)>> 8)  ;
			   ibuf[4] = ((s_time->tm_hour * 60)+ s_time->tm_min) & 0xFF;
			   ibuf[5] = s_time->tm_sec ;
			for (i=0; i<6; i++)
      {
        sprintf(tempo," [%d] %X ",i, ibuf[i]);
        DEBUG_PRINT(tempo);
      }
			DEBUG_PRINT(" \n");
            delay(TIME_BEFORE_SEND);
      radio.send(ibuf[1],(const void*)&ibuf[2],4,0);
      delay(TIME_AFTER_SEND);

		break ;

		case SET_PROGRAMME :  DEBUG_PRINT("Set sequence ");
  {
		int index1,op1,data1,op2,data2;


    gline.setFile (file, 3); // Min read 6

   if( ( gline.getPos()) < 0)
    {
      DEBUG_PRINT("pas de fichier "+file);
      break;
    }


		index1=0;
    while ((gline.getLine (tempo)))
     		{
	    		if( 2 != sscanf(tempo, "%x %x\n", &op1,&data1))
				break;
      		if( 2 != sscanf(tempo, "%x %x\n",&op2,&data2))
				break;
      		sprintf(tempo,"index %d op1 %X data1 %X op2 %X data2 %X \n",index1,op1,data1,op2,data2);
          DEBUG_PRINT(tempo);
			   ibuf[0] = 0 ;
  			 ibuf[2] = SET_CHRONO;
 			   ibuf[3] = (uint8_t) index1+64;
			   ibuf[4] = (uint8_t) op1;
			   ibuf[5] = (uint8_t) data1;
			   ibuf[6] = (uint8_t) op2;
			   ibuf[7] = (uint8_t) data2;


		for (i=0; i<8; i++)
    {
      sprintf(tempo," [%d] %X ",i, ibuf[i]);
      DEBUG_PRINT(tempo);
    }
		  DEBUG_PRINT(" \n");
            delay(TIME_BEFORE_SEND);
      radio.send(ibuf[1],(const void*)&ibuf[2],6,0);
	//	 status=write(rfm12_fd, ibuf, 8);
	//	 printf("W %d\n",status);
   		//  usleep (TIME_AFTER_SEND);
        delay(TIME_AFTER_SEND);
		  index1+= 1;
    		 }
		DEBUG_PRINT("nombre de sequence ");
    DEBUG_PRINTLN(index1);
		break ;
  }

	  default :

		for (i=0; i<len; i++)
    {
      sprintf(tempo," [%d] %X ",i, ibuf[i]);
      DEBUG_PRINT(tempo);
    }
			  DEBUG_PRINT("Ordre inconnu \r\n");
	//		  continue ;

} // end sxitvh
/*		while (ibuf[len-1] != '\n' )
			{ //try to synchronize
			len =read(fd_rfm12_in, &ibuf[0], 1);
    } */
		//usleep(3);

               }
	}




// thread for reading from fm

void getRfm(volatile uint8_t *obuf,volatile int len,volatile uint8_t senderID)

{
      char  tempo[50];
      uint8_t ibuf[RF69_MAX_DATA_LEN+1];
      int i;
      time_t tt;
   struct  tm *s_time;

 //----------------------- lecture rfm12 ------------------------------------//

     if (len > 0) {           // something to read

	   		//time (&tt);
//        timeClient.update();
        tt= timeClient.getUnixTime();
	         	sprintf(tempo,"%s", ctime(&tt));
            DEBUG_PRINT(tempo);
	         	sprintf(tempo,"\n %d bytes read Hexa ", len);
           DEBUG_PRINT(tempo);
	         	for (i=0; i<len; i++) {
	            		DEBUG_PRINT2(obuf[i],HEX);
                  DEBUG_PRINT(" ");
		         			}
	         	DEBUG_PRINT(" Dec ");
	         	for (i=0; i<len; i++) {
	            		DEBUG_PRINT2(obuf[i],DEC);
                  DEBUG_PRINT(" ");
	         			}
	  		DEBUG_PRINT("\n");

	// Send information to Domoticz
	//exemple
	// 	CURL *session = curl_easy_init();

//	curl_easy_setopt(session, CURLOPT_POSTFIELDS, "type=command&param=switchlight&idx=7&switchcmd=On");
//	curl_easy_setopt(session, CURLOPT_URL, "http://127.0.0.1:8080/json.htm?type=command&param=switchlight&idx=7&switchcmd=On");
//	curl_easy_perform(session);
//	curl_easy_cleanup(session);


//---------------------------------------------------demande de mise a l'heure --------------------//

			if ( obuf[0] == GET_TIME )
				{

				   DEBUG_PRINT("Set time ");
		  			//time (&tt);
//            timeClient.update();
            tt= timeClient.getUnixTime();
				s_time = localtime(&tt);
				ibuf[1] = jee_id_all ; // for all
				ibuf[0]= 0 ;
				ibuf[2] = SET_TIME;
	  		ibuf[3] = (s_time->tm_wday<< 4 )|( ((s_time->tm_hour * 60)+ s_time->tm_min)>> 8)  ;
				ibuf[4] = ((s_time->tm_hour * 60)+ s_time->tm_min) & 0xFF;
				ibuf[5] = s_time->tm_sec ;

				for (i=0; i<6; i++)
            			{	sprintf(tempo," [%d] %X ",i, ibuf[i]);
                      DEBUG_PRINT(tempo);
                    }

			//	sleep(5);
            delay(TIME_BEFORE_SEND);
       radio.send(jee_id_all,(const void*)&ibuf[2],4,0);
			   //	status=write(rfm12_fd, ibuf, 6);
			   //	printf("W %d\n",status);
				//usleep(3000);
       delay(TIME_AFTER_SEND);
				}
			if ( obuf[0] == ANSWER ) // 7
				{
			switch( obuf[1]	 )
				{
				case READ_BIT : // 5
// set loopback mode

				buffeur_antiboucle[ibuffeur_antiboucle].card=senderID ;
				buffeur_antiboucle[ibuffeur_antiboucle].bit=obuf[2] ;
				buffeur_antiboucle[ibuffeur_antiboucle].etat=obuf[3] ;
				buffeur_antiboucle[ibuffeur_antiboucle].time=tt ;

        printf (" antiboucle %d - %d %d %d %d \n\r",ibuffeur_antiboucle,buffeur_antiboucle[ibuffeur_antiboucle].card,buffeur_antiboucle[ibuffeur_antiboucle].bit,buffeur_antiboucle[ibuffeur_antiboucle].etat,buffeur_antiboucle[ibuffeur_antiboucle].time);
        ibuffeur_antiboucle +=1 ;
				if (ibuffeur_antiboucle == ANTI_BOUCLE_LEN )
					ibuffeur_antiboucle=0 ;
				event_domoticz_event_bit(senderID,obuf[2],obuf[3]);
				break;
				case GET_TEMP :
				event_domoticz_event_temp(senderID,obuf[2]);
				break;
				}

      }
    }
}
