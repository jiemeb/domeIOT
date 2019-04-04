
commandArray = {}

--local a ,b,c
local command
local file = io.open("/etc/event_domoticz.ini", "r")
 tbllines = {}
local i = 0
--  while true 
--do
--  a,b,c=file:read("*n","*n","*n")
--  if a == nil then break end
--  print( a..' '..b..' '..c..' ')
while true do
        ligne = file:read("*l")
        if ligne == nil then break end
 --       print (ligne)
t={}
i=0
for token in string.gmatch(ligne, "[^%s]+") do
        t[i]=tonumber(token,16)
        i=i+1
end
if t[1] < 255 
	then
	command = [[/bin/echo -e "\x]]..string.format("%x",t[0])..[[\x0\x5\x]]..string.format("%x",t[1])..[[" >>/tmp/rfm12b_in.fifo ]]
	print (command )
	else
	command = [[/bin/echo -e "\x]]..string.format("%x",t[0])..[[\x0\xF" >>/tmp/rfm12b_in.fifo ]]
	print (command )
	end
os.execute(command)
end
file:close()


--os.execute([[/bin/echo -e "\x2\x0\x5\x2" >/tmp/rfm12b_in.fifo ]] );  -- Read cardd 30 bit 0
--os.execute([[/bin/echo -e "\x2\x0\x5\x1" >/tmp/rfm12b_in.fifo ]] )  -- Read cardd 30 bit 1
--os.execute([[/bin/echo -e "\x2\x0\xF" >/tmp/rfm12b_in.fifo ]] )  -- Read cardd 30 
--	os.execute([[/bin/echo -e "\x1E\x0\x5\x2" >/tmp/rfm12b_in.fifo ]] );  -- Read cardd 30 bit 0
--	os.execute([[/bin/echo -e "\x1E\x0\x5\x1" >/tmp/rfm12b_in.fifo ]] )  -- Read cardd 30 bit 1
--	os.execute([[/bin/echo -e "\x1E\x0\xF" >/tmp/rfm12b_in.fifo ]] )  -- Read cardd 30 
 
return commandArray
