#include <Arduino.h>
#include <string.h>
#include "private.h"
#include "constant.h"


// ***************************************************************************************
// Here we find.
// Server dispatcher
// Stand alone to get information about my Tiny
// can
// **********************************************************************************

// Get information from Internet
extern void setupFSBrowser(void);
extern  void transformOrder ( volatile uint8_t  *ibuf, int len, String file) ;
// thread for reading from fm
extern void getRfm(volatile uint8_t  *obuf, int len, volatile uint8_t senderID) ;

extern void Blink(byte PIN, byte DELAY_MS, byte loops);
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
//#include <RFM69_ATC.h>     //get it here: https://github.com/lowpowerlab/RFM69
#include <SPI.h>           //included with Arduino IDE (www.arduino.cc)
//#include <LowPower.h>      //get library from: https://github.com/lowpowerlab/lowpower
#include <crc16.h>			// Compatible with RFM12 except Network ID

//****************************************************************************************************************
//**** IMPORTANT RADIO SETTINGS - YOU MUST CHANGE/CONFIGURE TO MATCH YOUR HARDWARE TRANSCEIVER CONFIGURATION! ****
//****************************************************************************************************************
#define LogSize 15
#define NETWORKID     212  //the same on all nodes that talk to each other
#define RECEIVER      0x2    //unique ID of the gateway/receiver
#define SENDER        0x1
#define NODEID        SENDER  //change to "SENDER" if this is the sender node (the one with the button)
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_433MHZ
#define ENCRYPTKEY    " 123456787654321" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Remove/comment if you have RFM69W!
//*****************************************************************************************************************************
//#define ENABLE_ATC      //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI        -75
//*********************************************************************************************
#define SERIAL_BAUD   112500

#ifdef __AVR_ATmega1284P__
#define LED           15 // Moteino MEGAs have LEDs on D15
#define BUTTON_INT    1 //user button on interrupt 1 (D3)
#define BUTTON_PIN    11 //user button on interrupt 1 (D3)
#else
#define LED           5 // Moteinos have LEDs on D9
#endif

#define USE_WDT

#ifdef ENABLE_ATC
RFM69_ATC radio;
#else
RFM69 radio(15, 4, false, 4);

//RFM69 radio(uint8_t slaveSelectPin=15, uint8_t interruptPin=4, bool isRFM69HW=false, uint8_t interruptNum=4);
#endif
String Log[LogSize];
int fifo_in ;
int fifo_out;


// Web Part
#include <ESP8266WiFi.h>

#include <EasyNTPClient.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include "event_domoticz.h"

// OTA + Debug


#include <ArduinoOTA.h>
#include <RemoteDebug.h>


WiFiUDP ntpUDP;
ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
EasyNTPClient timeClient(ntpUDP, "fr.pool.ntp.org", 7200, 360000);
RemoteDebug Debug;

void connectToWifi()
{  // Wait for WIFI connection
    WiFi.mode(WIFI_STA);
  int toggle = 0 ;
  int retry;
  do
  {
    if ( toggle & 1 )
    {
      WiFi.begin(WIFI_ESSID, WIFI_PASSWORD);
      toggle = 0;
    }
    else
    {
      WiFi.begin(WIFI_ESSID1, WIFI_PASSWORD1);
      toggle = 1;
    }

    retry = 15;
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
#ifdef USE_WDT
    wdt_reset();
#endif
      if ( (retry--) < 0)
        break;
    }
    Serial.print(".");
  } while (retry < 0);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP to access the server
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Debut de programme \n\r");
  Serial.println("Initialisation RFM69B \n\r");
  fifo_in = fifo_out = 0;

#ifdef USE_WDT
  wdt_enable(WDTO_8S);
#endif

  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(0);				//no encryption
  radio.promiscuous(false);	//
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif
  // passage a 9600  JM
  radio.writeReg(0x03, 0x0D);  //REG_BITRATEMSB: 9600 (0x000D, see DS p20)
  radio.writeReg(0x04, 0x0D);  //REG_BITRATELSB: 9600 (0x0005, see DS p20)

  radio.writeReg(0x05, 0x05);  //RF_FDEVMSB_90000
  radio.writeReg(0x06, 0xC3);  //RF_FDEVLSB_90000

  // Fin JM
  setupFSBrowser(); // Initailisation File
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  Serial.flush();
  radio.readAllRegs();
  pinMode(LED, OUTPUT);
  Blink(LED, 500, 3);
  // Get time offset in fileName
  File  file = SPIFFS.open("/time.ini", "r");
  int timeOfset = 3600 ;
  if (file != NULL)
  {
    uint8_t line[40 ];

    if (file.read( &line[0], 1) != 0)
    {
      if (line[0] == '2')
        timeOfset = 7200 ;
      Serial.println("File 7200");

    }
    Serial.println("pas 2");
    file.close();
  }

  timeClient.setTimeOffset (timeOfset);
  sprintf(buff, "Heure decallée de %d\n", timeOfset );
  Serial.println(buff);
  //
  //  attachInterrupt(BUTTON_INT, handleButton, FALLING);
  // Web server setup
  connectToWifi();
  server.on("/get" , getLog ); // Associate the handler function to the path
  server.on("/order", order);   //Associate the handler function to the path
  server.begin();
  event_domoticz_init();                                     //Start the server
  Serial.println("Server listening");

  // OTA
  ArduinoOTA.setHostname(NAME_GATEWAY); // on donne une petit nom a notre module
  ArduinoOTA.begin(); // initialisation de l'OTA
  ArduinoOTA.setPassword("celine");
  Debug.begin(NAME_GATEWAY);
}

WiFiClient client;

void loop() {
  // Handel OTA et Debug
  ArduinoOTA.handle();
  Debug.handle();

#ifdef USE_WDT
  wdt_reset();
#endif
  if (WiFi.status() != WL_CONNECTED) {
    // we've lost the connection, so we need to reconnect
        if (client) {
       client.stop();
     }
#ifdef DEBUG
    DEBUG_PRINTLN("connection WiFI");
#endif
    connectToWifi();
  }
  // Handle request
  server.handleClient();    //Handling of incoming requests
  delay(300);
  //Blink(LED, 500, 3);
  if (!digitalRead(LED))
  {
    delay(10);
    digitalWrite(LED, 1);
  }

  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    //print message received to serial
    DEBUG_PRINT("Rec ->");
    DEBUG_PRINT('['); DEBUG_PRINT(radio.SENDERID); DEBUG_PRINT("] ");

    char c; char cTab[4] ; String stringCurrent;
    stringCurrent = "";
    c = (radio.SENDERID & 0xF0) >> 4;
    cTab[0] = ((c > 9 ) ? c + 'A' - 10 : c + '0');
    c = (radio.SENDERID & 0x0F);
    cTab[1] = ((c > 9 ) ? c + 'A' - 10 : c + '0');
    cTab[2] = '-';
    cTab[3] = 0;
    stringCurrent += cTab;
    for (int i = 0; i < radio.DATALEN; i++)
    {
      DEBUG_PRINT2(radio.DATA[i], HEX);
      DEBUG_PRINT(" ");
      // store message in hexdecimal


      c = (radio.DATA[i] & 0xF0) >> 4;
      cTab[0] = ((c > 9 ) ? c + 'A' - 10 : c + '0');
      c = (radio.DATA[i] & 0x0F);
      cTab[1] = ((c > 9 ) ? c + 'A' - 10 : c + '0');
      cTab[2] = 0;
      stringCurrent += cTab;
      stringCurrent += " ";
    }
    //Crc16 crc;


    //  Log[fifo_in] += String(&radio.DATA,radio.DATALEN);

    stringCurrent += "\n";
    Log[fifo_in++] = stringCurrent;

    if ( fifo_in > (LogSize - 1))
      fifo_in = 0;

    DEBUG_PRINT("crc = 0x");
    DEBUG_PRINTLN2(radio.crc, HEX);
    DEBUG_PRINT("   [RX_RSSI:");DEBUG_PRINT(radio.RSSI);DEBUG_PRINT("]");
    DEBUG_PRINTLN(radio.DATALEN);
    digitalWrite(LED, 0); //debug

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    getRfm(radio.DATA, radio.DATALEN, radio.SENDERID);
  }


  radio.receiveDone(); //put radio in RX mode

  //Serial.print(".");
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  //  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON); //sleep Moteino in low power mode (to save battery)
}

void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i = 0; i < loops; i++)
  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}

void getLog() {

  String message = "";
  while (fifo_out != fifo_in)
  {
    message += Log[fifo_out++];
    if (fifo_out > LogSize - 1)
      fifo_out = 0;
  }


  server.send(200, "text/plain", message);          //Returns the HTTP response

}

void order() {
  uint8_t *p_message ;
  uint8_t length_message ;
  String messageR = "";

  messageR = server.arg("order");

  if (messageR == "") {    //Parameter not found

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
    transformOrder((uint8_t *) p_message, length_message, file);

    delete [] p_message ;
  }

  server.send(200, "text/plain", messageR);          //Returns the HTTP response

}
