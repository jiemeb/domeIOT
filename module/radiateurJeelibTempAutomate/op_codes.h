

#define EM	0x7F 			/* Fin de module		*/
#define BM	0x0			/* Debut de module		*/
#define IN	0x1			/* Entree bit dans Logi. Reg	*/
#define CI	0x2			/* Entree Complementer dans LCR	*/
#define AN	0x3			/* ET logique entre Bit et LCR	*/
#define CA	0x4			/* ET Complementer  Bit et LCR	*/
#define OR	0x5			/* OU logique entre Bit et LCR	*/
#define CO	0x6			/* OU Complementer  Bit et LCR	*/
#define XO	0x7			/* OU Exclusif entre Bit et LCR	*/
#define LI	0x8			/* Forcage du LCR             	*/
#define SB	0x9			/* Mise a 1 bit si LCR = 1 	*/
#define RB	0xA			/* Mise a 0 bit si LCR = 0	*/
#define OT	0xB			/* LCR -> Bit                	*/
#define LM	0xC			/* si LCR=0  retour sur -data	*/
#define TC	0xD			/* Attente sur Temps		*/
#define BU	0xE			/* Branchement sans condittion	*/
#define BT	0xF			/* Branchement si LCR = 1	*/
#define CB	0x10			/* COMPARE et Branche		*/
#define BF	0x11			/* Branchement si LCR = 0	*/
#define BE	0x12			/* Branchement sur Fin		*/
#define BS	0x13			/* Branchement fonction		*/
#define TR	0x14			/* Attente sur Temps registre	*/
#define TI	0x15			/* Attente sur Temps registre	*/
#define IT	0x16			/* Test fin de tempo sur TI	*/
#define IL	0x17			/* Complemente le LCR   	*/
#define TX	0x18			/* Attente sur Temps registre	*/

#define GM	0x1d			/* go module			*/
#define RT	0x1e			/* retour fonction		*/
#define NO	0x1f			/* 	pas d operation		*/

/* operation sur registre	*/

#define	LC	0x30		/*	Load 	Accu 	immediat	*/
#define	LR	0x31		/*	Load 	Accu	register	*/
#define	SR	0x32		/*	Store	Accu 	 Register	*/
#define	CC	0x33		/*	Compare Immediat Accu		*/
#define	CR	0x34		/*	Compare	Accu     Register	*/
#define	AD	0x35		/*	Add	Accu	 Register	*/
#define	SU	0x36		/*	Substr	Accu	 Register	*/
#define	MU	0x37		/*	Mul	Accu	 Register	*/
#define	DI	0x38		/*	Divi	Accu	 Register	*/
#define	AC	0x39		/*	Add	Accu	 Register	*/
#define	SC	0x3A		/*	Substr	Accu	 Register	*/
#define	MC	0x3B		/*	Mul	Accu	 Register	*/
#define	DC	0x3C		/*	Divi	Accu	 Register	*/
#define	CU	0x3D		/*	Count	Up	 Register	*/
#define	CD	0x3E		/*	Count	Down	 Register	*/
#define	BC	0x3F		/*	Branch	Conditt. Register	*/
#define	LX	0x40		/*	Load Accu indirect register	*/
#define	SX	0x41		/*	Store	Accu indirect  Register	*/
#define	SI	0x42		/*	Set register Index  Constant  	*/
#define	RI	0x43		/*	Reset register Index 		*/
#define	IR	0x44		/*	Set register Index         	*/
#define	IA	0x45		/*	Set register Index  sur ACCU   	*/
#define	LA	0x46		/*	Set register With Temperature  	*/


/* definitions des codes d impressions */

#define PM	0x50		/* Impression Message			*/
#define PL	0x51		/* Impression Date et Heure		*/
#define	EP	0x52		/* acquisition decimal dans reg		*/
#define	PR	0x53		/* Impression Message Indexe sur reg    */

/* definition des codes synchro taches		*/

#define	RE	0x60		/* redemarrage tache			*/
#define	PS	0x61		/* Arret pour Attente Synchro		*/
#define	BP	0x62		/* Arret pour Attente Synchro Point d arret */

/* Test ACR 		*/

#define	FE	0x68		/* LCR = 1 if ACR = 0			*/
#define	FS	0x69		/* LCR = 1 if ACR = 1		        */
#define	FI	0x6A		/* LCR = 1 if ACR = 2                   */


/* definitions des codes de tests 	*/

//#define EQ	0
//#define GE	6
//#define GT	1
//#define LE	5
//#define LT	2
//#define MI	2
//#define NE	4
