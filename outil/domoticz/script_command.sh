#!/bin/bash

echo  "TEST script" 
 
if [ "$1" == "0" ]
then
curl  http://192.168.0.10/order?order=030$2020$3  # Size of mess, Add, Order, Value  
else
curl  http://192.168.0.10/order?order=030$2030$3  
#/bin/echo -e "\x0$2\x0\x3\x$3" >>/tmp/rfm12b_in.fifo 
fi
exit 0
