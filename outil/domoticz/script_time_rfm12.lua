
commandArray = {}

--t = os.date('*t')
nowTable = os.date('*t')
nowMin = nowTable.min
print ( nowMin )
if ( nowMin %2 == 0) 
then
command = [[/home/domoticz/domoticz/scripts/lua/script_time.sh & ]]
os.execute(command)
print (commande)
end
--file:close()
 
return commandArray
root@jm-BT3-Pro:/home/domoticz/domoticz/scripts/lua# cat script_time.sh 
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
