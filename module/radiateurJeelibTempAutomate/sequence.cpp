/*----------------------------------------------------------------------*/
/* 		Programme VMAUT release le 13/05/93			*/
/*		Vers 5.1 pour Psos+ by JMB              */
/* now for attiny 			*/
/*		JMB		Completly rewrited					*/
/*----------------------------------------------------------------------*/

#include <JeeLib.h>

#include <EEPROM.h>
#include "automate.h"
#include "sequence.h"
#include "op_codes.h"

//#include <SoftwareSerial.h>
//extern SoftwareSerial Serial; // RX, TX
extern Automate A;


//#define SDEBUG
//#define SDEBUG1

void sequenceur::setup()
{
  char j ;
	unsigned char i ;

	for ( j = 0,i = 0;j < SEQUENCE_SIZE ; j += SIZE_INSTRUCTION)
	{
		if (EEPROM.read(SEQUENCE+j)== BM) // Find Begin
		{
		module[i].lcr = 1;
		module[i].aad = 0;
		module[i].acr = 0;
		module[i].time = 0;
		module[i].pas_courant = 0;
		module[i].pas = 0xff;
		for (short origine = (j+SEQUENCE); j < SEQUENCE_SIZE ; j += SIZE_INSTRUCTION)
			{

			if (EEPROM.read(SEQUENCE+j)== EM)
				{
				module[i].pas = origine;
#ifdef SDEBUG1
 Serial.print((F("S I ")));
 Serial.println(origine,DEC);
#endif
        i++;
				break;
				}

			}

		if (j == SEQUENCE_SIZE  || i >= NOMBRE_MODULE)
				break ;
		}
	        else
                {
                    break ;
                }
	}


}

void sequenceur::live()
{

for (unsigned char i = 0 ;( i < NOMBRE_MODULE && module[i].pas != 0xFF ) ; i++)
	{
	short pas_module = module[i].pas ;
        if (module[i].time > 0 )
              {
                module[i].time--;
                continue;
              }
	char stay_on = 1 ;
#ifdef SDEBUG
 Serial.print((F("S B ")));
 Serial.println(i,DEC);
#endif


	while (stay_on ) // loop on module
	{
        char data = EEPROM.read(module[i].pas_courant+pas_module+1);  // get data of order
 #ifdef SDEBUG
 Serial.print((F("S Bc ")));
 Serial.println(EEPROM.read(module[i].pas_courant+pas_module),HEX);
 Serial.println(data,HEX);
#endif
	switch (EEPROM.read(module[i].pas_courant+pas_module))              // switch order
		{
		case EM:                     // End
    case BE:                    // Got to End
		module[i].pas_courant = 0  ;// at the end should be zero
		stay_on = 0;
		continue ;
		case BM:
		module[i].lcr = 1;
		module[i].aad = 0;
		module[i].acr = 0;
		module[i].time = 0;
		break;

    case IN:
		module[i].lcr = A.readBit(data) ;       /* Entree bit dans Logi. Reg	*/
		break;

    case CI:
		module[i].lcr = ~A.readBit(data)& 0x1;  /* Entree Complementer dans LCR	*/
		break;

    case AN:
		module[i].lcr &= A.readBit(data);       /* ET logique entre Bit et LCR	*/
		break;

    case CA:
		module[i].lcr &= ~A.readBit(data)& 0x1; /* ET Complementer  Bit et LCR	*/
		break;

    case OR:
		module[i].lcr |= A.readBit(data);       /* OU logique entre Bit et LCR	*/
		break;
    case CO:
		module[i].lcr |= ~A.readBit(data) & 0x1; /* OU Complementer  Bit et LCR	*/
		break;

    case XO:
		module[i].lcr  ^= A.readBit(data) & 0x1; /* OU Exclusif entre Bit et LCR	*/
		break;

    case LI:
		module[i].lcr  = 0x1; /* Forcage du LCR             	*/
		break;

    case SB:              /* Mise a 1 bit si LCR = 1 	*/
		if (module[i].lcr)
         A.setBit (data);
		break;

    case RB:              /* Mise a 0 bit si LCR = 1	*/
		if (module[i].lcr)
       A.resetBit (data);
		break;

    case OT:              /* Mise a 0 bit si LCR = 1	*/
		if (module[i].lcr)
      A.setBit (data);
    else
      A.resetBit (data);
		break;
/* Inform Master   */

    case I1:              /*Send master Bit set si LCR = 1	*/
    if (module[i].lcr)
      A.send (1,data);
    break;

    case I0:            /*Send master Bit reset  si LCR = 1	*/
    if (module[i].lcr)
      A.send (0,data);
    break;


    
    case IC:
     data = temptxIn.temp / 10 ;     /* Load  Accu  immediat  */
     A.send (2,data);     
    break;

/*   Time and Jump */

    case LM:              /* si LCR=0  retour sur -data	*/
		if (module[i].lcr == 0)
    {
 		module[i].pas_courant = 0  ;
		stay_on = 0;
		continue ;
    }
		break;

    case TC:
    if (module[i].lcr)
    {
		module[i].time  = data; /* Attente sur Temps		*/
    stay_on = 0;
		}
    break;

    case BU:
		module[i].pas_courant  = (data*SIZE_INSTRUCTION) -SIZE_INSTRUCTION; /* Branchement sans condittion	*/
		break;
    case BT:
		if (module[i].lcr)
                        module[i].pas_courant  = (data*SIZE_INSTRUCTION) -SIZE_INSTRUCTION; //* Branchement si LCR = 1	*/
		break;

    case BF:
		if (module[i].lcr == 0)
                        module[i].pas_courant  = (data*SIZE_INSTRUCTION) -SIZE_INSTRUCTION; //* Branchement si LCR = 0	*/
		break;

    case IL:
		module[i].lcr = ~module[i].lcr & 1 ;                     /* Complemente le LCR   	*/
		break;


/* Arithmétique */

    case CC:
		module[i].acr =(module[i].aad > data) ? 1 : (module[i].aad < data) ? 2 : 0 ; /*	Compare Immediat Accu		*/
		break;

		case FS:                                /* Put LCR = 1 if ACR = 2  Supe    */
		module[i].lcr = (module[i].acr == 1) ? 1 : 0  ;
		break;

		case FE:                                /* Put LCR = 1 if ACR = 0  Equal      */
		module[i].lcr = (module[i].acr == 0) ? 1 : 0  ;
		break;

		case FI:                                /* Put LCR = 1 if ACR = 2  Infe    */
		module[i].lcr = (module[i].acr == 2) ? 1 : 0  ;
		break;

    case AC:
		module[i].aad += data ;                  /*	Add	Accu	 immediat	*/
    module[i].acr =(module[i].aad > 0) ? 1 : (module[i].aad < 0) ? 2 : 0 ;
		break;

    case SC:
		module[i].aad -= data ;                   /*	Subs 	Accu 	immediat	*/
    module[i].acr =(module[i].aad > 0) ? 1 : (module[i].aad < 0) ? 2 : 0 ;
		break;

    case DC:
		module[i].aad /= data ;                   /*	Div 	Accu 	immediat	*/
    module[i].acr =(module[i].aad > 0) ? 1 : (module[i].aad < 0) ? 2 : 0 ;
		break;

    case MC:
		module[i].aad *= data ;                   /*	Multi 	Accu 	immediat	*/
    module[i].acr =(module[i].aad > 0) ? 1 : (module[i].aad < 0) ? 2 : 0 ;
		break;

    case LA:
    if (data & 0x80 )
		      module[i].aad = temptxIn.temp / 10 ;     /*	Load 	Accu 	immediat	*/
  //            module[i].acr =(module[i].aad > 0) ? 1 : (module[i].aad < 0) ? 2 : 0 ;
		break;
//#define	LR	0x31		/*	Load 	Accu	register	*/
//#define	SR	0x32		/*	Store	Accu 	 Register	*/

//#define	CR	0x34		/*	Compare	Accu     Register	*/
//#define	AD	0x35		/*	Add	Accu	 Register	*/
//#define	SU	0x36		/*	Substr	Accu	 Register	*/
//#define	MU	0x37		/*	Mul	Accu	 Register	*/
//#define	DI	0x38		/*	Divi	Accu	 Register	*/



//#define	CU	0x3D		/*	Count	Up	 Register	*/
//#define	CD	0x3E		/*	Count	Down	 Register	*/
//#define	BC	0x3F		/*	Branch	Conditt. Register	*/
//#define	LX	0x40		/*	Load Accu indirect register	*/
//#define	SX	0x41		/*	Store	Accu indirect  Register	*/
//#define	SI	0x42		/*	Set register Index  Constant  	*/
//#define	RI	0x43		/*	Reset register Index 		*/
//#define	IR	0x44		/*	Set register Index         	*/
//#define	IA	0x45		/*	Set register Index  sur ACCU   	*/

//#define CB	0x10		/* COMPARE et Branche		*/
		}
		module[i].pas_courant += SIZE_INSTRUCTION ;
                if (module[i].pas_courant  > SEQUENCE_SIZE )
                {
                module[i].pas_courant = 0  ;// at the end should be zero
		stay_on = 0;
                }
		}
#ifdef SDEBUG
 Serial.print((F("End M ")));
 Serial.println(i,DEC);
#endif


	}

#ifdef SDEBUG
 Serial.print((F("End S ")));
#endif

}
