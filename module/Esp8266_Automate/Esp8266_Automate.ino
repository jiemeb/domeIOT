/*
  ESP8266 mDNS serial wifi bridge by Daniel Parnell 2nd of May 2015
*/

//#define BONJOUR_SUPPORT
#include <Arduino.h>
#include <ESP8266WiFi.h>
#ifdef BONJOUR_SUPPORT
#include <ESP8266mDNS.h>
#endif

#include <ESP8266WebServer.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>

extern void order();
extern void getLog();
//#include "esp8266_pwm.h"

#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <RemoteDebug.h>
#define DEBUG

#include "automate.h"
#include  "sendResult.h"
#include  "event_domoticz.h"

// Extern Object
extern void setupFSBrowser(void);
RemoteDebug Debug;

//#define DEBUG_BEGIN(x)    debugSerialTX.begin(x)
#define DEBUG_PRINT(x)    Debug.print(x)
#define DEBUG_PRINTF(x,y)   Debug.printf(x,y)
#define DEBUG_PRINTLN(x)  Debug.println(x)
#define DEBUG_WRITE(x,y)    Debug.write(x,y)


// application config

// comment out the following line to enable DHCP
//#define STATIC_IP

#ifdef STATIC_IP
// change the IP address and gateway to match your network
#define IP_ADDRESS "192.168.0.210"
#define GATEWAY_ADDRESS "192.168.0.254"
#define NET_MASK "255.255.255.0"
#endif

// change the WiFi info to match your network
#define WIFI_SSID "ici_maison"
#define WIFI_PASSWORD "E18E38980D"
#define WIFI_SSID1 "ici_maison_EXT"
#define WIFI_PASSWORD1 "E18E38980D"
#define BAUD_RATE 57600
#define TCP_LISTEN_PORT 80
#define USE_WDT

// if the bonjour support is turned on, then use the following as the name
// define for OTA et DEBUG
#define DEVICE_NAME "Esp_Automate1"

// serial end ethernet buffer size
#define BUFFER_SIZE 128

// hardware config
//#define WIFI_LED 5

#ifdef BONJOUR_SUPPORT
// multicast DNS responder
MDNSResponder mdns;
#endif

WiFiUDP ntpUDP;
EasyNTPClient timeClient(ntpUDP, "fr.pool.ntp.org", 7200, 360000);
uint8_t cardID;
#define LogSize 15
String Log[LogSize];
int fifo_in ;
int fifo_out;
Automate A;
ESP8266WebServer server(TCP_LISTEN_PORT);


#ifdef STATIC_IP
IPAddress parse_ip_address(const char *str) {
  IPAddress result;
  int index = 0;

  result[0] = 0;
  while (*str) {
    if (isdigit((unsigned char)*str)) {
      result[index] *= 10;
      result[index] += *str - '0';
    } else {
      index++;
      if (index < 4) {
        result[index] = 0;
      }
    }
    str++;
  }

  return result;
}

#endif

void connect_to_wifi() {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);


#ifdef STATIC_IP
  IPAddress ip_address = parse_ip_address(IP_ADDRESS);
  IPAddress gateway_address = parse_ip_address(DEVICE_NAME);
  IPAddress netmask = parse_ip_address(NET_MASK);

  WiFi.config(ip_address, gateway_address, netmask);
#endif

  // Wait for WIFI connection
  int toggle = 0 ;
  int retry;
  do
  {
    if ( toggle & 1 )
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      toggle =0;
    }
    else
    {
      WiFi.begin(WIFI_SSID1, WIFI_PASSWORD1);
      toggle = 1;
    }

    retry = 100;
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      if ( (retry--) < 0)
        break;
    }
    #ifdef USE_WDT
        wdt_reset();
    #endif
  } while (retry < 0);


}

void error() {
  int count = 0;
#ifdef DEBUG
  DEBUG_PRINTLN("connection Erreur");
#endif

  delay(1000);

}

void setup(void)
{

/*  digitalWrite(WIFI_LED, LOW);
  pinMode(WIFI_LED, OUTPUT); */
  fifo_in = fifo_out = 0;
  Serial.begin(BAUD_RATE);
  Serial.println("\n\rDebut de programme \n\r");

  #ifdef USE_WDT
    wdt_enable(WDTO_8S);
  #endif
  // Connect to WiFi network
  connect_to_wifi();

  // Setup OTA

  ArduinoOTA.setHostname(DEVICE_NAME);                  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.begin();

  // initialisation de la librairie de debug



  DEBUG_PRINT("Ready for OTA on ");
  DEBUG_PRINT("IP address:\t");
  DEBUG_PRINTLN(WiFi.localIP());


#ifdef BONJOUR_SUPPORT
  // Set up mDNS responder:
  if (!mdns.begin(DEVICE_NAME, WiFi.localIP())) {
    error();
  }
#endif
   // Start TCP server
  server.on("/get" , getLog ); // Associate the handler function to the path
  server.on("/order", order);   //Associate the handler function to the path

  server.begin();

    Debug.begin(DEVICE_NAME);
    setupFSBrowser();
    event_domoticz_init();
   A.setup();
}





void loop(void)
{
  int bytes_read;
  uint8_t net_buf[BUFFER_SIZE+1];
  uint8_t serial_buf[BUFFER_SIZE+1];
  #ifdef USE_WDT
    wdt_reset();
  #endif
  ArduinoOTA.handle(); // test to get update
  Debug.handle();

  if (WiFi.status() != WL_CONNECTED) {
    // we've lost the connection, so we need to reconnect

#ifdef DEBUG
    DEBUG_PRINTLN("connection WiFI");
#endif

    connect_to_wifi();
  }

  server.handleClient();    //Handling of incoming requests
  delay(300);

#ifdef BONJOUR_SUPPORT
  // Check for any mDNS queries and send responses
  mdns.update();
#endif
// Start here specifique

}

void order() {
  uint8_t *p_message ;
  uint8_t length_message ;
  String messageR = "";

  messageR = server.arg("order");

  if (messageR == "") {    //Parameter not found
   DEBUG_PRINT( "Pas Ordre" );
    messageR = "order not found";

  } else {
    uint8_t k ;
    uint8_t length_message = messageR.length() / 2;
    p_message = new  uint8_t [10];
    k = 0;    //Parameter found
    for (uint8_t  i = 0 ; i < messageR.length(); i++ )
    {
      uint8_t j;

      j = messageR [i];
      if (j >= 'a' && j <= 'f' ) j &= 0x5F  ; // convert minuscule majuscule
      if ( j >= 'A' && j <= 'F' ) j = j - 'A' + 10;
      else if (j >= '0' && j <= '9') j = j - '0';
      else break;
      if ( i & 0x1)
      {
        p_message [i / 2] = k + j;
      }
      else
      {
        k = j * 16 ;
      }

    }
    String file = "";

    file = server.arg("file");

    DEBUG_PRINT( messageR );
//    A.decodeMessage(p_message, length_message, file);
    transformOrder(p_message, length_message, file);
   Log[fifo_in++] = messageR+"\n";

    if ( fifo_in > (LogSize - 1))
      fifo_in = 0;
    delete [] p_message ;
  }

  server.send(200, "text/plain", messageR);          //Returns the HTTP response
}

void getLog() {

  String message = "";
  while (fifo_out != fifo_in)
  {
    message += Log[fifo_out++];
    if (fifo_out > LogSize - 1)
      fifo_out = 0;
  }

message += "\n\r" + WiFi.SSID() + " (" + WiFi.RSSI()+ ")" ;

  server.send(200, "text/plain", message);          //Returns the HTTP response

}
