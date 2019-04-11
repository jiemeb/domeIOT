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
