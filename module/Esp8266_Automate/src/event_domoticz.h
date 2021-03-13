
#define SWITCH "/json.htm?type=command&param=switchlight&idx=%d&switchcmd=%s"
#define TEMP  "/json.htm?type=command&param=udevice&idx=%d&nvalue=0&svalue=%d"

#define sendToHost "192.168.0.25"
#define sendToPort 8080
#define numberCardMax 29
extern void event_domoticz_init();
extern void event_domoticz_event_temp(char card,char value);
extern void event_domoticz_event_bit(char card,char bit,char value);
extern void event_domoticz_end();
