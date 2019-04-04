#!/bin/bash

echo  "TEST script" 
 
if [ "$1" == "0" ]
then
/bin/echo -e "\x0$2\x0\x2\x$3" >>/tmp/rfm12b_in.fifo 
else
/bin/echo -e "\x0$2\x0\x3\x$3" >>/tmp/rfm12b_in.fifo 
fi
exit 0

# if(devicechanged['2.2']=='On')then
#--	os.execute([[/bin/echo -e "\x02\x0\x3\x2" >/tmp/rfm12b_in.fifo ]])  -- Set Bit card 30 bit 0
# --  end


#--   if(devicechanged['2.2']=='Off')then
#--	os.execute([[/bin/echo -e "\x02\x0\x2\x2" >/tmp/rfm12b_in.fifo ]] )  -- Reset Bit card 30 bit 0
#--   end

#--   if(devicechanged['2.1']=='On')then
#--	os.execute([[/bin/echo -e "\x02\x0\x3\x1" >/tmp/rfm12b_in.fifo ]])  -- Set Bit card 30 bit 0
#--   end

#--   if(devicechanged['2.1']=='Off')then
#--	os.execute([[/bin/echo -e "\x02\x0\x2\x1" >/tmp/rfm12b_in.fifo ]])  -- Reset Bit card 30 bit 
 
 
 
 
#return commandArray
