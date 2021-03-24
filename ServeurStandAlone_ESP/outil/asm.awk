BEGIN {print"assemblage";FS="\t"

op_code["EM"]="7F" 			#/* Fin de module		*/
op_code["BM"]="00"			#/* Debut de module		*/
op_code["IN"]="01"			#/* Entree bit dans Logi. Reg	*/
op_code["CI"]="02"			#/* Entree Complementer dans LCR	*/
op_code["AN"]="03"			#/* ET logique entre Bit et LCR	*/
op_code["CA"]="04"			#/* ET Complementer  Bit et LCR	*/
op_code["OR"]="05"			#/* OU logique entre Bit et LCR	*/
op_code["CO"]="06"			#/* OU Complementer  Bit et LCR	*/
op_code["XO"]="07"			#/* OU Exclusif entre Bit et LCR	*/
op_code["LI"]="08"			#/* Forcage du LCR             	*/
op_code["SB"]="09"			#/* Mise a 1 bit si LCR = 1 	*/
op_code["RB"]="0A"			#/* Mise a 0 bit si LCR = 0	*/
op_code["OT"]="0B"			#/* LCR -> Bit                	*/
op_code["LM"]="0C"			#/* si LCR=0  retour sur -data	*/
op_code["TC"]="0D"			#/* Attente sur Temps		*/
op_code["BU"]="0E"			#/* Branchement sans condittion	*/
op_code["BT"]="0F"			#/* Branchement si LCR = 1	*/
op_code["CB"]="10"			#/* COMPARE et Branche		*/
op_code["BF"]="11"			#/* Branchement si LCR = 0	*/
op_code["BE"]="12"			#/* Branchement sur Fin		*/
op_code["BS"]="13"			#/* Branchement fonction		*/
op_code["TR"]="14"			#/* Attente sur Temps registre	*/
op_code["TI"]="15"			#/* Attente sur Temps registre	*/
op_code["IT"]="16"			#/* Test fin de tempo sur TI	*/
op_code["IL"]="17"			#/* Complemente le LCR   	*/
op_code["TX"]="18"			#/* Attente sur Temps registre	*/

#/* operation sur registre	*/

op_code["LC"]="30"	#/*	Load 	Accu 	immediat	*/
op_code["LR"]="31"	#/*	Load 	Accu	register	*/
op_code["SR"]="32"	#/*	Store	Accu 	 Register	*/
op_code["CC"]="33"	#/*	Compare Immediat Accu		*/
op_code["CR"]="34"	#/*	Compare	Accu     Register	*/
op_code["AD"]="35"	#/*	Add	Accu	 Register	*/
op_code["SU"]="36"	#/*	Substr	Accu	 Register	*/
op_code["MU"]="37"	#/*	Mul	Accu	 Register	*/
op_code["DI"]="38"	#/*	Divi	Accu	 Register	*/
op_code["AC"]="39"	#/*	Add	Accu	 Register	*/
op_code["SC"]="3A"	#/*	Substr	Accu	 Register	*/
op_code["MC"]="3B"	#/*	Mul	Accu	 Register	*/
op_code["DC"]="3C"	#/*	Divi	Accu	 Register	*/
op_code["CU"]="3D"	#/*	Count	Up	 Register	*/
op_code["CD"]="3E"	#/*	Count	Down	 Register	*/
op_code["BC"]="3F"	#/*	Branch	Conditt. Register	*/
op_code["LX"]="40"	#/*	Load Accu indirect register	*/
op_code["SX"]="41"	#/*	Store	Accu indirect  Register	*/
op_code["SI"]="42"	#/*	Set register Index  Constant  	*/
op_code["RI"]="43"	#/*	Reset register Index 		*/
op_code["IR"]="44"	#/*	Set register Index         	*/
op_code["IA"]="45"	#/*	Set register Index  sur ACCU   	*/
op_code["LA"]="46"	#/*	Set register With Temperature  	*/


#/* definitions des codes d impressions */

op_code["PM"]="50"	#/* Impression Message			*/
op_code["PL"]="51"	#/* Impression Date et Heure		*/
op_code["EP"]="52"	#/* acquisition decimal dans reg		*/
op_code["PR"]="53"	#/* Impression Message Indexe sur reg    */

#/* definition des codes synchro taches		*/

op_code["RE"]="60"	#/* redemarrage tache			*/
op_code["SP"]="61"	#/* Arret pour Attente Synchro		*/
op_code["BP"]="62"	#/* Arret pour Attente Synchro Point d arret */

#/* Test ACR 		*/

op_code["FE"]="68"	#/* LCR = 1 if ACR = 0			*/
op_code["FS"]="69"	#/* LCR = 1 if ACR = 1		        */
op_code["FI"]="6A"	#/* LCR = 1 if ACR = 2                   */
NL=0
PASS=0
system(" > \"sequence\"")
}

{ 
while (PASS==0)  # traitement des LABELs
{
if((getline<FILENAME) > 0)
{
if ($1 != "")
	{#print" LABEL "  $1 " " NL
	LABEL[$1] = NL
 }
 if ($2!="")  
  {
	if ($2 == "EM") 
	{ NL=0 }
	else
	{ NL++ }
  }  

NRU++

}
else
{
PASS=1
print ("NB Ligne ",NRU )
status = close(FILENAME)
getline $0
}
}


  if ($2!="")  
  { OP = op_code[$2] 
	if ($3 ~ ":") 
	{ DATA = LABEL[$3] }
	else 
	{ DATA =  $3 }
if (OP == "" || DATA == "")
	print ("Erreur ligne " FNR)
  print ( OP " " DATA ) >> "sequence"
	}  
}

END{

print"C'est fini"}

