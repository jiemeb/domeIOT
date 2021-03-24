/*
  ESP8266 mDNS serial wifi bridge by Daniel Parnell 2nd of May 2015
*/

//#define BONJOUR_SUPPORT

#include <WiFi.h>
#ifdef BONJOUR_SUPPORT
#include <mDNS.h>
#endif
#include "constant.h"
#include <WiFiClient.h>

//#include "esp8266_pwm.h"

#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <RemoteDebug.h>
#define DEBUG

extern RemoteDebug Debug;
extern void connectToWifi(void);


// application config




// change the WiFi info to match your network
#define WIFI_SSID "ici_maison"
#define WIFI_PASSWORD "E18E38980D"
#define WIFI_SSID1 "ici_maison_EXT"
#define WIFI_PASSWORD1 "E18E38980D"
#define BAUD_RATE 57600
#define TCP_LISTEN_PORT 9999



// serial end ethernet buffer size
#define BUFFER_SIZE 128

// hardware config
#define WIFI_LED 2
#define TX_PIN 17
#define RX_PIN 16


#ifdef BONJOUR_SUPPORT
// multicast DNS responder
MDNSResponder mdns;
#endif

WiFiServer serverSerial(TCP_LISTEN_PORT);

void error() {
  int count = 0;
#ifdef DEBUG
  DEBUG_PORT.println("connection Erreur");
#endif

  delay(1000);

}

void setupWifiSerial(void)
{

  digitalWrite(WIFI_LED, LOW);
  pinMode(WIFI_LED, OUTPUT);
  Serial2.begin(BAUD_RATE,SERIAL_8N1,RX_PIN,TX_PIN);

   // Start TCP serverSerial
  serverSerial.begin();
}

WiFiClient clientSerial;


void loopWifiSerial(void)
{
  int bytes_read;
  uint8_t net_buf[BUFFER_SIZE+1];
  uint8_t serial_buf[BUFFER_SIZE+1];


 if (WiFi.status() != WL_CONNECTED) {
    // we've lost the connection, so we need to reconnect
     if (clientSerial) {
       clientSerial.stop();
      }
#ifdef DEBUG
    DEBUG_PORT.println("connection WiFI");
#endif

    connectToWifi();
  }

  // Check if a clientSerial has connected
  if (!clientSerial) {
    // eat any bytes in the serial buffer as there is nothing to see them
    while (Serial2.available()) {
      Serial2.read();
    }
    clientSerial = serverSerial.available();
  }

  if (clientSerial.connected()) {
    // check the network for any bytes to send to the serial
 //   int count = clientSerial.available();
 //   if (count > 0) {
   int count ;
   while( (count = clientSerial.available())) {
      bytes_read = clientSerial.read(net_buf, min(count, BUFFER_SIZE));
      net_buf[count] = 0 ;
#ifdef DEBUG
      DEBUG_PORT.printf("RecNet %s ",net_buf);
#endif

      Serial2.write(net_buf, bytes_read);
      Serial2.flush();
    }



  while( (count = Serial2.available())) {



    for ( bytes_read=0 ;  bytes_read < (min(count, BUFFER_SIZE)) ; bytes_read++ )
        serial_buf[bytes_read] = Serial2.read ();

    if (bytes_read > 0) {
      clientSerial.write(serial_buf, bytes_read);
      serial_buf[bytes_read] = 0;
#ifdef DEBUG
      DEBUG_PORT.printf("SendNet %s ",serial_buf);
#endif
      clientSerial.flush();
                      }
  }

  }
 else {
     clientSerial.stop();
  }

}
