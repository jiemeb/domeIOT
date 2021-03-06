#include "private.h"
#define SWITCH "/json.htm?type=command&param=switchlight&idx=%d&switchcmd=%s"
#define TEMP  "/json.htm?type=command&param=udevice&idx=%d&nvalue=0&svalue=%d"

#define sendToHost SERVER_DOMOTICZ
#define sendToPort 8080

extern void event_domoticz_init();
extern void event_domoticz_event_temp(char card,char value);
extern void event_domoticz_event_bit(char card,char bit,char value);
extern void event_domoticz_end();
