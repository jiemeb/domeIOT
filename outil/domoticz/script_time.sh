#!/bin/bash
set -x
while read card bit event
do
card=0x$card
bit=0x$bit
echo $card $bit event
if [ $event -eq  0 ]
then 
continu
fi
if [ "$bit" = "0xFF"  ]
 then
chaineCurl=`printf 'curl http://192.168.0.10/order?order=01%02X0F' $card`
echo $chaineCurl
$chaineCurl
else
chaineCurl=`printf 'curl http://192.168.0.10/order?order=02%02X05%02X' $card $bit`
echo $chaineCurl
$chaineCurl
#curl http://192.168.0.10/order?order=02$card05$bit
fi
sleep 1
done < /etc/event_domoticz.ini
 
exit 0
