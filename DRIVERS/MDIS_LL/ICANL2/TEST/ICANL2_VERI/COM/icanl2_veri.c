/****************************************************************************
 ************                                                    ************
 ************                   ICANL2_VERI                      ************
 ************                                                    ************
 ****************************************************************************
 *
 *       Author: kp
 *        $Date: 2010/03/08 13:45:11 $
 *    $Revision: 1.10 $
 *
 *  Description: Verification program for ICANL2 MDIS4 driver
 *
 *  Requires two ICANL2 devices controlled from the same CPU
 *	(1x double M65 or 2xP5)
 *
 *     Required: libraries: mdis_api, icanl2_api, usr_utl, usr_oss
 *     Switches: -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_veri.c,v $
 * Revision 1.10  2010/03/08 13:45:11  amorbach
 * R: All tests failed
 * M: startByte setting in VerifyMultiFrames() corrected
 *
 * Revision 1.9  2009/06/29 15:58:01  CRuff
 * R: 1. Signal handler may cause stack overflow
 *    2.compiler warnings
 * M: 1. added Macro __MAPILIB to Signal handler declaration /implementation
 *    2.fixed compiler warings caused by type conversions etc.
 *
 * Revision 1.8  2004/04/05 08:59:35  ub
 * globals declared static
 *
 * Revision 1.7  2003/02/06 13:36:35  ub
 * Added: Set size if event fifo to 32 entries.
 *
 * Revision 1.6  2002/02/07 14:15:45  ub
 * REV 1.4
 * TestI(): test of transmission of single frames added
 * TestM(): removed from default test list due to insufficient reliability of
 * generating BusOff event. Call manually.
 *
 * Revision 1.5  2002/02/04 15:16:25  ub
 * REV 1.3
 * Added: TestL(): Signals
 * Added: TestM(): BusOff detection and recovery
 *
 * Revision 1.4  2001/12/19 09:10:16  kp
 * REV 1.2
 * TestB(): Don't check Tx fifo info (CPU speed dependent)
 * TestB(): wait only 0.3s for events
 * TestD(): timestamp checking more tolerant (CPU speed dependent)
 * TestG(): UOS_MikroDelay() between WriteFifo calls to simulate slower CPU
 *
 * Revision 1.3  2001/12/12 16:24:32  kp
 * cosmetics
 *
 * Revision 1.2  2001/12/12 14:44:46  ub
 * TestH(): Now removes SIGLVL event from queue.
 * TestD(): Checks time stamps of ICANL2API_NewestMsg().
 * TestB(): Tests ICANL2API_FifoInfo().
 *
 * Revision 1.1  2001/11/29 12:00:21  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: icanl2_veri.c,v 1.10 2010/03/08 13:45:11 amorbach Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>

#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_drv.h>
#include <MEN/icanl2_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define REV "1.4"				/* program revision */

#define CHK(expression) \
 if( !(expression)) {\
	 printf("\n*** Error during: %s\nfile %s\nline %d\n", \
      #expression,__FILE__,__LINE__);\
      printf("%s\n",ICANL2API_ErrorToString(UOS_ErrnoGet()));\
     goto abort;\
 }

#define CHK2(expression) \
 if( !(expression)) {\
	 printf("\n*** Error during: %s\nfile %s\nline %d\n", \
      #expression,__FILE__,__LINE__);\
     goto abort;\
 }
#define ICANL2_TCLK 5.33

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/

/* globals for each device */
typedef struct {
	MDIS_PATH path;
	char *name;
	int32 obj[20];
} DEVICE;


/* test list description */
typedef struct {
	char code;
	char *descr;
	int (*func)(DEVICE *d1, DEVICE *d2);
} TEST_ELEM;

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static int TestA( DEVICE *d1, DEVICE *d2 );
static int TestB( DEVICE *d1, DEVICE *d2 );
static int TestC( DEVICE *d1, DEVICE *d2 );
static int TestD( DEVICE *d1, DEVICE *d2 );
static int TestE( DEVICE *d1, DEVICE *d2 );
static int TestF( DEVICE *d1, DEVICE *d2 );
static int TestG( DEVICE *d1, DEVICE *d2 );
static int TestH( DEVICE *d1, DEVICE *d2 );
static int TestI( DEVICE *d1, DEVICE *d2 );
static int TestJ( DEVICE *d1, DEVICE *d2 );
static int TestK( DEVICE *d1, DEVICE *d2 );
static int TestL( DEVICE *d1, DEVICE *d2 );
static int TestM( DEVICE *d1, DEVICE *d2 );
static void printmsg( int level, char *fmt, ... );
static int SetupDev1( DEVICE *d, int baud, int useExtended );
static int SetupDev2( DEVICE *d, int baud, int useExtended );

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static TEST_ELEM G_testList[] = {
	{ 'a', "Normal frame xmit, Polled recv", TestA },
	{ 'b', "Normal frame xmit, blocked recv", TestB },
	{ 'c', "Normal frame xmit with Tx complete events", TestC },
	{ 'd', "newest msg", TestD },
	{ 'e', "Overrun xmit fifo", TestE },
	{ 'f', "Overrun recv fifo", TestF },
	{ 'g', "Global/General object", TestG },
	{ 'h', "Simultaneous Rx/Tx", TestH },
	{ 'i', "Out-of-band Tx", TestI },
	{ 'j', "Remote transmit frames", TestJ },
	{ 'k', "Cyclic Tx/Timestamping", TestK },
	{ 'l', "Signals", TestL },
	{ 'm', "BusOff detection and recovery", TestM },
	{ 0, NULL, NULL }
};

static int G_verbose, G_baud, G_useExtended=FALSE;

/********************************* usage ************************************
 *
 *  Description: Print program usage
 *
 *---------------------------------------------------------------------------
 *  Input......: -
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static void usage(void)
{
	TEST_ELEM *te=G_testList;

	printf("Usage: icanl2_veri [<opts>] <device1> <device2> [<opts>]\n");
	printf("Function: Test program for ICANL2 MDIS4 driver\n");
	printf("          Requires two ICANL2 devices on the same bus and CPU\n");
	printf("Options:\n");
	printf("  device1      first device name............... [none]    \n");
	printf("  device2      first device name............... [none]    \n");
	printf("  -v=<num>     verbosity level (0-2)........... [0]       \n");
	printf("  -n=<runs>    number of runs through all tests [1]\n");
	printf("  -e           use extended (29bit ID) mode ... [basic]\n");
	printf("  -b=<baud>    use bitrate of <baud> in Hz ...  [1000000]\n");
	printf("  -s           stop on first error ............ [no]     \n");
	printf("  -t=<list>    perform only those tests listed: [all]     \n");

	while( te->func ){
		printf("    %c: %s\n", te->code, te->descr );
		te++;
	}

	printf("\n");
	printf("(c) 2001 by MEN mikro elektronik GmbH. Revision %s\n\n", REV);
}

/********************************* main *************************************
 *
 *  Description: Program main function
 *
 *---------------------------------------------------------------------------
 *  Input......: argc,argv	argument counter, data ..
 *  Output.....: return	    success (0) or error -1
 *  Globals....: -
 ****************************************************************************/
int main(int argc, char *argv[])
{
	int32	n;
	char	*str, buf[40], *errstr, *testlist;
	int runs, run, errCount=0, err, stopOnFirst;
	DEVICE d[2];
	TEST_ELEM *te;
	char *tCode;

	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("v=?n=t=b=es", buf))) {	/* check args */
		printf("*** %s\n", errstr);
		return(1);
	}

	if (UTL_TSTOPT("?")) {						/* help requested ? */
		usage();
		return(1);
	}

	/*--------------------+
    |  get arguments      |
    +--------------------*/
	d[0].name = NULL;
	d[1].name = NULL;

	for (n=1; n<argc; n++){
		if (*argv[n] != '-') {
			if( d[0].name == NULL )
				d[0].name = argv[n];
			else if( d[1].name == NULL )
				d[1].name = argv[n];
			else
				usage();
		}
	}

	if (d[1].name == NULL) {
		usage();
		return(1);
	}

	G_verbose = ((str = UTL_TSTOPT("v=")) ? atoi(str) : 0);
	testlist  = ((str = UTL_TSTOPT("t=")) ? str : "abcdefghijkl"/*mnopqrstuvxyz"*/);
	runs	  = ((str = UTL_TSTOPT("n=")) ? atoi(str) : 1);
	G_baud	  = ((str = UTL_TSTOPT("b=")) ? atoi(str) : 1000000);
	stopOnFirst = !!UTL_TSTOPT("s");
	G_useExtended = !!UTL_TSTOPT("e");

	/*-----------------+
	|  Setup devices   |
	+-----------------*/
	if( SetupDev1( &d[0], G_baud, G_useExtended ))
		return 1;
	if( SetupDev2( &d[1], G_baud, G_useExtended ))
		return 1;
	UOS_MikroDelayInit();

	/*-----------------+
	|  Perform tests   |
	+-----------------*/
	for( tCode=testlist; *tCode; tCode++ ){

		for( te=G_testList; te->func; te++ )
			if( *tCode == te->code )
				break;

		if( te->func == NULL ){
			printf("Unknown test: %c\n", *tCode );
			return 1;
		}

		for( run=1; run<=runs; run++ ){
			printf("=== Performing test %c: %-43s (Run %d/%d) ",
				   te->code, te->descr, run, runs );
			printmsg( 1, "===\n");
			fflush(stdout);

			err = te->func( &d[0], &d[1] );
			errCount += err;

			printmsg( 1, "Test %c: ", te->code);
			printf( "%s\n", err ? "FAILED" : "ok" );

			if( err && stopOnFirst )
				goto abort;
		}
	}
 abort:
	printf("------------------------------------------------\n");
	printf("TEST RESULT: %d errors\n", errCount );

/* 	printf( "(Hit <RETURN>)\n" ); getchar(); */

	M_close( d[0].path );
	M_close( d[1].path );

	return errCount ? 1 : 0 ;
}

static void printmsg( int level, char *fmt, ... )
{
	va_list ap;

	if( level <= G_verbose ){
		va_start(ap,fmt);
		vprintf( fmt, ap );
		va_end(ap);
	}
}


static int VerifyFrameData( ICANL2_DATA *msg, u_int8 dlen,
							u_int8 startByte )
{
	int i;

	if( dlen != 0xff ){
		CHK2( dlen == msg->data_len );
	}
	else
		dlen = msg->data_len;

	for(i=0; i<dlen; i++ )
		CHK2( msg->data[i] == startByte++);

	return startByte;
 abort:
	return -1;
}

static int VerifyMultiFrames( ICANL2_DATA *msg, int nMsg,
							  u_int8 dLen, u_int8 startByte)
{
	int sb=startByte;

	while( nMsg-- ){
		if( (sb = VerifyFrameData( msg, dLen, (u_int8)sb )) < 0 )
			return -1;
		msg++;
	}
	return sb;
}

static u_int8 BuildFrameData( ICANL2_DATA *msg, u_int32 id,
							  u_int8 dLen, u_int8 startByte )
{
	int i;

	for(i=0; i<dLen; i++ )
		msg->data[i] = startByte++;

	msg->data_len = dLen;
	msg->id = id;
	msg->tx_flags = 0;
	return startByte;

}

static u_int8 BuildMultiFrames(ICANL2_DATA *msg, u_int32 id, int nMsg,
							   u_int8 dLen, u_int8 startByte)
{
	while( nMsg-- ){
		startByte = BuildFrameData( msg, id, dLen, startByte );
		msg++;
	}
	return startByte;
}

static void DumpFrame( ICANL2_DATA *msg )
{
	int i;

	printmsg( 2, "Frame: id=0x%x dl=%d data=", msg->id, msg->data_len);
	for( i=0; i<msg->data_len; i++ )
		printmsg( 2, " %02x", msg->data[i]);
	printmsg( 2, "\n");
}

static int SetTiming( DEVICE *d, int baud, u_int8 busconf )
{
	const struct {
		int baud;
		u_int8 brp, sjw, tseg1, tseg2, spl;
	} tpar[] = {
		{ 1000000,        0,     1,       4,       1,    0 },
		{  500000,        1,     1,       4,       1,    0 },
		{  250000,        3,     1,       4,       1,    0 },
		{  125000,        7,     1,       4,       1,    0 },
		{  100000,        9,     1,       4,       1,    0 },
#if 0
		/* some tests will not work below 100kBit/s (timeouts) */
		{   50000,       19,     1,       4,       1,    0 },
		{   20000,       49,     1,       4,       1,    0 },
		{  	10000,       39,     1,       13,      4,    0 },
#endif
		{ 0 }
	};
	int i;

	for( i=0; tpar[i].baud; i++ )
		if( tpar[i].baud == baud )
			break;

	if( tpar[i].baud == 0 ){
		printf("Baudrate %d unknown\n", baud );
		return 1;
	}


	CHK( ICANL2API_SetTiming( d->path,
							  tpar[i].brp,
							  tpar[i].sjw,
							  tpar[i].tseg1,
							  tpar[i].tseg2,
							  tpar[i].spl,
							  busconf 			/* BusConf */
		) == 0);

	return 0;
 abort:
	return 1;
}

/********************************* SetupDev1 *********************************
 *
 *  Description: Sets up first CAN device
 *
 *  Creates the following objects:
 *
 *	Idx 	Id(Hex)	Type	Fifo	SigLvl
 *	0		101		Tx		50	    -
 *	1		201		Tx		32 		-
 *	2		301		Tx		20	   	-
 *	3		302		Tx		500	   	-
 *	4		401		Tx		10		-
 *	5		701		Rx		64		20
 *	6		702		Tx/RTR	-		-
 *---------------------------------------------------------------------------
 *  Input......: d			device handle
 *				 baud		baud rate
 *				 useExtended extended mode
 *  Output.....: returns	0=ok -1=error
 *  Globals....: -
 ****************************************************************************/
static int SetupDev1( DEVICE *d, int baud, int useExtended )
{
	printmsg( 1, "Setting up device %s\n", d->name );

	d->path = -1;
	CHK((d->path = M_open(d->name)) >= 0);

	/*--------------------+
    |  config             |
    +--------------------*/

	if( SetTiming( d, baud, 0x00 ) ) return 1;

	/* set acceptence mask and operation mode */
	if( useExtended ){
		CHK( ICANL2API_SetAccMask( d->path, 0x3FFFFFFF, 1 ) == 0 );
	}
	else {
		CHK( ICANL2API_SetAccMask( d->path, 0x7FF, 0 ) == 0 );
	}


	printmsg( 2, "Creating objects\n");

	CHK( (d->obj[0] = ICANL2API_CreateObject(
		d->path,
		0x101, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		50, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[1] = ICANL2API_CreateObject(
		d->path,
		0x201, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		32, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[2] = ICANL2API_CreateObject(
		d->path,
		0x301, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		20, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[3] = ICANL2API_CreateObject(
		d->path,
		0x302, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		500, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[4] = ICANL2API_CreateObject(
		d->path,
		0x401, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		10, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[5] = ICANL2API_CreateObject(
		d->path,
		0x701, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		64, 					/* fifo size */
		20						/* signal level */
		)) >= 0 );

	CHK( (d->obj[6] = ICANL2API_CreateObject(
		d->path,
		0x702, 					/* id */
		ICANL2_OBJT_RTR,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		0,	 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	{
		int i;
		for(i=0;i<7;i++)
			printmsg(3,"obj[%d] is %d\n", i, d->obj[i] );
	}
	/* enable can communication */
	printmsg( 2, "Enable can communication\n");
	CHK( ICANL2API_EnableCan( d->path, 1 ) == 0 );

	return 0;
 abort:
	if( d->path >= 0 ) M_close( d->path );
	return -1;
}


/********************************* SetupDev2 *********************************
 *
 *  Description: Sets up second CAN device
 *
 *  Creates the following objects:
 *
 *	Idx 	Id(Hex)	Type	Fifo	SigLvl
 *	0		-		Global	256		0
 *	1		101		Rx		32	    0
 *	2		201		Rx		256 	1
 *	3		301		Rx		1024   	20
 *	4		302		Rx		0	   	-
 *	5		701		Tx		64		-
 *	6		702		Rx		1		1
 *---------------------------------------------------------------------------
 *  Input......: d			device handle
 *				 baud		baud rate
 *				 useExtended extended mode
 *  Output.....: returns	0=ok -1=error
 *  Globals....: -
 ****************************************************************************/
static int SetupDev2( DEVICE *d, int baud, int useExtended )
{
	printmsg( 1, "Setting up device %s\n", d->name );

	d->path = -1;
	CHK((d->path = M_open(d->name)) >= 0);

	/*--------------------+
    |  config             |
    +--------------------*/

	if( SetTiming( d, baud, 0x00 ) ) return 1;

	/* set acceptence mask and operation mode */
	if( useExtended ){
		CHK( ICANL2API_SetAccMask( d->path, 0x3FFFFFFF, 1 ) == 0 );
	}
	else {
		CHK( ICANL2API_SetAccMask( d->path, 0x7FF, 0 ) == 0 );
	}


	printmsg( 2, "Creating objects\n");

	CHK( (d->obj[0] = ICANL2API_CreateObject(
		d->path,
		0,	 					/* id */
		ICANL2_OBJT_GLOBAL,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		256, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[1] = ICANL2API_CreateObject(
		d->path,
		0x101, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		32, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[2] = ICANL2API_CreateObject(
		d->path,
		0x201, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		256, 					/* fifo size */
		1						/* signal level */
		)) >= 0 );

	CHK( (d->obj[3] = ICANL2API_CreateObject(
		d->path,
		0x301, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		1024, 					/* fifo size */
		20						/* signal level */
		)) >= 0 );

	CHK( (d->obj[4] = ICANL2API_CreateObject(
		d->path,
		0x302, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		0,	 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[5] = ICANL2API_CreateObject(
		d->path,
		0x701, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_TRANSMIT, 	/* mode */
		64, 					/* fifo size */
		0						/* signal level */
		)) >= 0 );

	CHK( (d->obj[6] = ICANL2API_CreateObject(
		d->path,
		0x702, 					/* id */
		ICANL2_OBJT_STD,	 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		1,	 					/* fifo size */
		1						/* signal level */
		)) >= 0 );

    /* resize event fifo */
    {
        int32 obj;
        CHK( ICANL2API_DeleteObject( d->path, 0 ) >= 0 );
    	CHK( (obj = ICANL2API_CreateObject(
    		d->path,
    		0, 					    /* id */
    		ICANL2_OBJT_EVENT,	 	/* type */
    		0, 	                    /* mode */
    		32,	 					/* fifo size */
    		0						/* signal level */
    		)) >= 0 );
    }
	{
		int i;
		for(i=0;i<7;i++)
			printmsg(3,"obj[%d] is %d\n", i, d->obj[i] );
	}
	/* enable can communication */
	printmsg( 2, "Enable can communication\n");
	CHK( ICANL2API_EnableCan( d->path, 1 ) == 0 );

	return 0;
 abort:
	if( d->path >= 0 ) M_close( d->path );
	return 1;
}

/* Normal frame xmit, Polled recv */
static int TestA( DEVICE *d1, DEVICE *d2 )
{
	#define TA_NMSG 20
	ICANL2_DATA txMsg[TA_NMSG];
	ICANL2_DATA rxMsg[TA_NMSG], *rx;
	int rcvd, nFrames;
	u_int32 startTime;

	/* send 20 frames to d1 obj[0] */
	BuildMultiFrames( txMsg, 0, TA_NMSG, 8, 0 );

	printmsg( 2, "sending frames\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[0], txMsg,
										 TA_NMSG )) > 0 );

	CHK2( nFrames == TA_NMSG );

	/* poll until all frames received by d2 */
	rcvd = 0;
	rx = rxMsg;
	startTime = UOS_MsecTimerGet();

	printmsg( 2, "waiting for frames\n");

	while( rcvd < TA_NMSG ){
		CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[1], rx, 10 ))
			 >= 0 );
		printmsg( 3, " got %d frames ", nFrames );
		rx += nFrames;
		rcvd += nFrames;

		printmsg( 3, " rcvd=%d\n", rcvd );

		CHK2((UOS_MsecTimerGet() - startTime) < 1000 );
	}

	/* verify frame data */
	if( VerifyMultiFrames( rxMsg, rcvd, 8, 0 ) < 0 ) goto abort;

	return 0;

 abort:
	return 1;
}


/* Normal frame xmit, blocked recv */
static int TestB( DEVICE *d1, DEVICE *d2 )
{
	#define TB_NMSG 20
	ICANL2_DATA txMsg[TB_NMSG];
	ICANL2_DATA rxMsg[TB_NMSG];
	int nFrames, event, sigLvlCount, n=0;
	u_int32 evObj, timeStamp;
	int32 numEntries;

	/* send frames to d1 obj[1] */
	BuildMultiFrames( txMsg, 0, TB_NMSG, 4, 0 );

	printmsg( 2, "sending frames d1 obj[1]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[1], txMsg,
										 TB_NMSG )) > 0 );
	CHK2( nFrames == TB_NMSG );

#if 0
	/* cannot use this since it is CPU speed dependent */
	CHK( ICANL2API_FifoInfo( d1->path, d1->obj[1], &numEntries ) == 0 );
	printmsg( 2, "d1->obj[1]: used fifo entries: %d\n", numEntries );
	CHK2( numEntries > 15 );
#endif
	/* wait for events on d2 obj[2]'s siglevel is 1 */
	printmsg( 2, "waiting for events on d2 obj[2]\n");

	/* wait 0.3s for a new event */
	sigLvlCount = 0;

	while( n<3 ){
		CHK( (event = ICANL2API_GetEvent( d2->path, 100, &evObj, &timeStamp))
			 >= 0 );

		if( event == ICANL2_EV_SIGLVL && evObj == d2->obj[2] )
			sigLvlCount++;

		if( event )
			printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
					  evObj);

		n++;
	}
	/* must have received exactly one fifo event */
	CHK2( sigLvlCount == 1 );

	/* fifo must contain now <TB_MSG> frames */
	CHK( ICANL2API_FifoInfo( d2->path, d2->obj[2], &numEntries ) == 0 );
	printmsg( 2, "d2->obj[2]: used fifo entries: %d\n", numEntries );
	CHK2( numEntries == TB_NMSG );

	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[2],
										rxMsg, TB_NMSG )) >= 0 );
	CHK2( nFrames == TB_NMSG );

	if( VerifyMultiFrames( rxMsg, TB_NMSG, 4, 0 ) < 0 ) goto abort;

	/*---- Now the same with a different fifo signal level ----*/

	/* send frames to d1 obj[2] */
	BuildMultiFrames( txMsg, 0, TB_NMSG, 1, 0x12 );

	printmsg( 2, "sending frames d1 obj[2]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg,
										 TB_NMSG )) > 0 );
	CHK2( nFrames == TB_NMSG );

	/* wait for events on d2 obj[3]'s siglevel is 20 */
	printmsg( 2, "waiting for events on d2 obj[3]\n");

	CHK( (event = ICANL2API_GetEvent( d2->path, 300, &evObj, &timeStamp))
		 >= 0 );

    printmsg( 2, "got event %s\n", ICANL2API_EventToString( event ) );
	CHK2( event == ICANL2_EV_SIGLVL );
	CHK2( evObj == d2->obj[3] );

	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[3],
										rxMsg, TB_NMSG )) >= 0 );
	CHK2( nFrames == TB_NMSG );

	if( VerifyMultiFrames( rxMsg, TB_NMSG, 1, 0x12 ) < 0 ) goto abort;


	return 0;

 abort:
	return 1;
}

/* Normal frame xmit with Tx complete events */
static int TestC( DEVICE *d1, DEVICE *d2 )
{
	#define TC_NMSG 20
	ICANL2_DATA txMsg[TC_NMSG];
	ICANL2_DATA rxMsg[TC_NMSG];
	int nFrames, event, n=0, evTxCount=0, evRxCount=0;
	u_int32 startTime, evObj, timeStamp;

	/* send frames to d1 obj[2] */
	BuildMultiFrames( txMsg, 0, TC_NMSG, 8, 0x13 );

	for( n=0; n<TC_NMSG; n++ )
		txMsg[n].tx_flags = ICANL2_FE_GENINT;

	printmsg( 2, "sending frames d1 obj[2]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg,
										 TC_NMSG )) > 0 );
	CHK2( nFrames == TC_NMSG );

	/* wait for events on d2 obj[3]'s siglevel is 20 */
	printmsg( 2, "waiting for events\n");

	startTime = UOS_MsecTimerGet();

	while( evTxCount < TC_NMSG || evRxCount < 1 ){

		CHK( (event = ICANL2API_GetEvent( d1->path, 10, &evObj, &timeStamp))
			 >= 0 );

		if( event ){
			printmsg( 3, "Event d1: %s obj %d\n",
					  ICANL2API_EventToString(event),
					  evObj);
			if( event == ICANL2_EV_TX_COMPLETE ){
				CHK2( evObj == d1->obj[2] );
				evTxCount++;
			}
		}


		CHK( (event = ICANL2API_GetEvent( d2->path, 0, &evObj, &timeStamp))
			 >= 0 );

		if( event ){
			printmsg( 3, "Event d2: %s obj %d\n",
					  ICANL2API_EventToString(event),
					  evObj);
			if( event == ICANL2_EV_SIGLVL ){
				CHK2( evObj == d2->obj[3] );
				evRxCount++;
			}
		}

		if( (UOS_MsecTimerGet() - startTime) > 1000 ){
			printmsg(1,"evTxCount=%d evRxCount=%d\n", evTxCount, evRxCount );
			CHK2(0);
		}
	}
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[3],
										rxMsg, TC_NMSG )) >= 0 );
	CHK2( nFrames == TC_NMSG );

	if( VerifyMultiFrames( rxMsg, TC_NMSG, 8, 0x13 ) < 0 ) goto abort;

	return 0;

 abort:
	return 1;
}


/* newest msg */
static int TestD( DEVICE *d1, DEVICE *d2 )
{
	#define TD_NMSG 100
	ICANL2_DATA txMsg[TD_NMSG];
	ICANL2_DATA rxMsg[1];
	int32 rv, msgCount=0;
	u_int32 startTime, lastTs;
	double dt, dtmin, dtmax, dtavg;

	/* check if the object is empty */
	CHK( (rv = ICANL2API_NewestMsg( d2->path, d2->obj[4], rxMsg )) == 1 );

	printmsg( 2, "sending two frames to different objects\n");
	/* send two frames to objects */
	BuildFrameData( txMsg, 0, 3, 0x11 );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg, 1 ));
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[3], txMsg, 1 ));
	UOS_Delay(20);				/* wait for frame transmit */

	/* now new frame must have been received */
	CHK( (rv = ICANL2API_NewestMsg( d2->path, d2->obj[3], rxMsg )) == 0 );

	if( VerifyFrameData( rxMsg, 3, 0x11 ) < 0 )
		goto abort;

	/* message is now empty */
	CHK( (rv = ICANL2API_NewestMsg( d2->path, d2->obj[3], rxMsg )) == 1 );

	printmsg( 2, "checking last received message\n");

	/* last received message must be identical with sended msg. */
	CHK( (rv = ICANL2API_NewestMsg( d2->path, ICANL2_GENERAL_HANDLE, rxMsg )) ==
		 0 );

	if( VerifyFrameData( rxMsg, 3, 0x11 ) < 0 )
		goto abort;

	/* rcvd id must match id of second sended frame */
	CHK( rxMsg[0].id == 0x302);

	/* message is now empty */
	CHK( (rv = ICANL2API_NewestMsg( d2->path, ICANL2_GENERAL_HANDLE, rxMsg )) ==
		 1 );

	/* anything in fifo ? */
 	CHK( ICANL2API_ReadFifo( d2->path, d2->obj[3], rxMsg, 1 ) );

	printmsg( 2, "sending multiple frames\n");

	/* send multiple frame to object */
	BuildMultiFrames( txMsg, 0, TD_NMSG, 8, 0x77 );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[3], txMsg, TD_NMSG ) ==
		 TD_NMSG);

	/* make sure that all received frames are consistent */
	startTime = UOS_MsecTimerGet();
	lastTs = 0;
	dtmin = 10000.0;
	dtmax = dtavg = 0.0;

	while( (UOS_MsecTimerGet() - startTime) < 300 ){
		CHK( (rv = ICANL2API_NewestMsg( d2->path, d2->obj[4], rxMsg )) >= 0 );
		if( rv == 0 ){
			msgCount++;
			if( VerifyFrameData( rxMsg, 0xff, rxMsg[0].data[0] ) < 0 ){
				DumpFrame( rxMsg );
				goto abort;
			}
			CHK( rxMsg[0].id == 0x302 );

			if( lastTs != 0 ){
				dt = (double)(rxMsg[0].time - lastTs) * ICANL2_TCLK;
				if( dt < dtmin ) dtmin = dt;
				if( dt > dtmax ) dtmax = dt;
				dtavg += dt;
				printmsg( 3, "ts=0x%08x dt=%3.2f\n", rxMsg[0].time, dt );
			}
			lastTs = rxMsg[0].time;
		}
	}
	dtavg /= msgCount;

	printmsg( 2, "%d different frames received\n", msgCount);
	printmsg( 2, "dt: max=%3.2f  min=%3.2f  avg=%3.2fus\n",
				dtmax, dtmin, dtavg );
	CHK( IN_RANGE( dtmin, 50, 1250 ) );
	CHK( IN_RANGE( dtmax, 50, 111000 ) ); /* some OSes might be slow */

	/* Send frames with zero length??? */

	return 0;

 abort:
	return 1;
}

/* Overrun xmit fifo */
static int TestE( DEVICE *d1, DEVICE *d2 )
{
	#define TE_NMSG 40
	ICANL2_DATA txMsg[TE_NMSG];
	ICANL2_DATA rxMsg[TE_NMSG];
	int nFrames, event;
	u_int32 evObj, timeStamp;

	/* send 40 frames to d1 obj[2] (has only 20 entries) */
	BuildMultiFrames( txMsg, 0, TE_NMSG, 8, 0x10 );

	printmsg( 2, "sending frames d1 obj[2]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg,
										 TE_NMSG )) > 0 );
	CHK2( nFrames == 20 );		/* cannot have more than 20 frames */

	/* wait for siglvl event on d2 */
	CHK( (event = ICANL2API_GetEvent( d2->path, 1000, &evObj, &timeStamp))
		 >= 0 );

    printmsg( 2, "got event %s\n", ICANL2API_EventToString( event ) );
	CHK2( event == ICANL2_EV_SIGLVL );
	CHK2( evObj == d2->obj[3] );

	/* test overflow recovery */
	BuildMultiFrames( txMsg, 0, 5, 8, 0x20 );

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg,
										 5 )) == 5 );
	UOS_Delay(100);

	/* read messages */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[3], rxMsg, 25 ))
		 == 25 );

	if( VerifyMultiFrames( rxMsg, 20, 8, 0x10 ) < 0 ) goto abort;
	if( VerifyMultiFrames( &rxMsg[20], 5, 8, 0x20 ) < 0 ) goto abort;

	return 0;

 abort:
	return 1;

}

/* Overrun recv fifo */
static int TestF( DEVICE *d1, DEVICE *d2 )
{
	#define TF_NMSG 40
	ICANL2_DATA txMsg[TF_NMSG];
	ICANL2_DATA rxMsg[TF_NMSG];
	int nFrames, event;
	u_int32 evObj, timeStamp;

	/* send 40 frames to d1 obj[0] */
	BuildMultiFrames( txMsg, 0, TF_NMSG, 7, 0x10 );
	txMsg[TF_NMSG-1].tx_flags |= ICANL2_FE_GENINT;

	printmsg( 2, "sending frames d1 obj[0]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[0], txMsg,
										 TF_NMSG )) == TF_NMSG );

	/* wait for tx complete */
	CHK( (event = ICANL2API_GetEvent( d1->path, 1000, &evObj, &timeStamp))
		 >= 0 );
	CHK( event == ICANL2_EV_TX_COMPLETE );
	UOS_Delay(20);

	/* check for a single fifo full event on d2 */
	CHK( (event = ICANL2API_GetEvent( d2->path, 0, &evObj, &timeStamp))
		 >= 0 );
	printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
			  evObj);
	CHK2( event == ICANL2_EV_FIFO_FULL );

	CHK( (event = ICANL2API_GetEvent( d2->path, 0, &evObj, &timeStamp))
		 >= 0 );
	CHK2( event == 0 );

	/* read messages from d2's obj[1] (has 32 entries only) */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[1], rxMsg, TF_NMSG))
		 >= 0 );
	printmsg( 2, "nFrames=%d\n", nFrames );
	CHK2( nFrames == 32 );


	/* test overflow recovery */
	BuildMultiFrames( txMsg, 0, 5, 8, 0x20 );

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, d1->obj[0], txMsg,
										 5 )) == 5 );
	UOS_Delay(100);

	/* read messages */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[1], &rxMsg[32], 5 ))
		 == 5 );

	if( VerifyMultiFrames( rxMsg, 32, 7, 0x10 ) < 0 ) goto abort;
	printmsg( 3, "first 32 frames verified ok\n");
	if( VerifyMultiFrames( &rxMsg[32], 5, 8, 0x20 ) < 0 ) goto abort;

	return 0;

 abort:
	return 1;

}

/* Global/General object */
static int TestG( DEVICE *d1, DEVICE *d2 )
{
	#define TG_NMSG 10
	ICANL2_DATA txMsg[TG_NMSG];
	ICANL2_DATA rxMsg[TG_NMSG];
	int nFrames, i;

	/* disable CAN communication */
	CHK( ICANL2API_EnableCan( d2->path, 0 ) == 0 );

	/* create general object */
	CHK( (d2->obj[19] = ICANL2API_CreateObject(
		d2->path,
		0,	 					/* id */
		ICANL2_OBJT_GENERAL, 	/* type */
		ICANL2_MOD_RECEIVE, 	/* mode */
		256,		 			/* fifo size */
		0						/* signal level */
		)) >= 0 );

	/* enable CAN communication */
	CHK( ICANL2API_EnableCan( d2->path, 1 ) == 0 );

	/*
	 * send objects: 10 frames to id 301, 10 frames to id 401
	 * therefore ID 301 must be received by the general fifo and obj idx 3
	 * ID 401 must be received by the general fifo and global fifo
	 * Due to the implicit priority, all ID 301 frames are sent first
	 */
	BuildMultiFrames( txMsg, 0, TG_NMSG, 4, 0x7A );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[2], txMsg, TG_NMSG ) ==
		 TG_NMSG);

	/*
	 * Add a small delay to simulate slow OS or CPU
	 * Using this delay, the firmware has already started to xmit the above
	 * FIFO. This small delay puts some extra complexity to the firmware's
	 * xmit handler (Firmware Version 1.1.1 had a bug concerning this issue)
	 */
	UOS_MikroDelay(400);

	BuildMultiFrames( txMsg, 0, TG_NMSG, 8, 0x22 );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[4], txMsg, TG_NMSG ) ==
		 TG_NMSG);

	UOS_Delay( 100 );

	/* check receive on obj ID=0x301 */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[3],
										rxMsg, TG_NMSG )) == TG_NMSG );
	if( VerifyMultiFrames( rxMsg, TG_NMSG, 4, 0x7a ) < 0 ) goto abort;

	/* check receive on global obj */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[0],
										rxMsg, TG_NMSG )) == TG_NMSG );
	if( VerifyMultiFrames( rxMsg, TG_NMSG, 8, 0x22 ) < 0 ) goto abort;
	/* check IDs */
	for( i=0; i<nFrames; i++ )
		CHK( rxMsg[i].id == 0x401 );

	/* check receive on general fifo */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[19],
										rxMsg, TG_NMSG )) == TG_NMSG );
	if( VerifyMultiFrames( rxMsg, TG_NMSG, 4, 0x7a ) < 0 ) goto abort;
	for( i=0; i<nFrames; i++ )
		CHK( rxMsg[i].id == 0x301 );

	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[19],
										rxMsg, TG_NMSG )) == TG_NMSG );
	if( VerifyMultiFrames( rxMsg, TG_NMSG, 8, 0x22 ) < 0 ) goto abort;
	for( i=0; i<nFrames; i++ )
		CHK( rxMsg[i].id == 0x401 );

	/* disable CAN communication */
	CHK( ICANL2API_EnableCan( d2->path, 0 ) == 0 );

	/* delete general object */
	CHK( ICANL2API_DeleteObject( d2->path, d2->obj[19] ) == 0 );

	/* enable CAN communication */
	CHK( ICANL2API_EnableCan( d2->path, 1 ) == 0 );


	return 0;

 abort:
	return 1;

}

/* Simultaneous Rx/Tx */
static int TestH( DEVICE *d1, DEVICE *d2 )
{
	#define TH_NMSG 30
	ICANL2_DATA txMsg[TH_NMSG];
	ICANL2_DATA rx1Msg[TH_NMSG], rx2Msg[TH_NMSG], *rx1, *rx2;
	int nFrames, rcvd1, rcvd2, event;
	u_int32 startTime, evObj, timeStamp, timeNeeded;

	/* Send device 1: ID 101 */
	BuildMultiFrames( txMsg, 0, TH_NMSG, 8, 0x44 );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[0], txMsg, TH_NMSG ) ==
		 TH_NMSG);

	/* Send device 2: ID 701 */
	BuildMultiFrames( txMsg, 0, TH_NMSG, 4, 0x22 );
	CHK( ICANL2API_WriteFifo( d2->path, d2->obj[5], txMsg, TH_NMSG ) ==
		 TH_NMSG);

	/* polled receive */
	rcvd1 = rcvd2 = 0;
	rx1 = rx1Msg;
	rx2 = rx2Msg;

	startTime = UOS_MsecTimerGet();
	timeNeeded = 0;

	while( rcvd1 < TH_NMSG || rcvd2 < TH_NMSG ){
		CHK( (nFrames = ICANL2API_ReadFifo( d1->path, d1->obj[5], rx1,
											TH_NMSG-rcvd1))
			 >= 0 );
		printmsg( 3, " d1: got %d frames ", nFrames );
		rx1 += nFrames;
		rcvd1 += nFrames;
		printmsg( 3, "rcvd1= %d \n", rcvd1 );

		CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[1], rx2,
											TH_NMSG-rcvd2))
			 >= 0 );
		printmsg( 3, " d2: got %d frames ", nFrames );
		rx2 += nFrames;
		rcvd2 += nFrames;
		printmsg( 3, "rcvd2= %d \n", rcvd2 );

		timeNeeded = UOS_MsecTimerGet() - startTime;
		CHK2( timeNeeded < 1000 );
	}

	printmsg( 1, "timeNeeded= %d \n", timeNeeded );

	/* verify frame data */
	if( VerifyMultiFrames( rx1Msg, rcvd1, 4, 0x22 ) < 0 ) goto abort;
	if( VerifyMultiFrames( rx2Msg, rcvd2, 8, 0x44 ) < 0 ) goto abort;

	/* get event */
	CHK( (event = ICANL2API_GetEvent( d1->path, 100, &evObj, &timeStamp)) >= 0 );

	printmsg( 1, "Event: %s obj %d\n", ICANL2API_EventToString(event),
			  evObj);

	return 0;

 abort:
	return 1;
}


/* Out-of-band Tx */
static int TestI( DEVICE *d1, DEVICE *d2 )
{
	/* Note: priority of OOB is not tested here */
	#define TI_NMSG 20
	ICANL2_DATA txMsg[TI_NMSG];
	ICANL2_DATA rxMsg[TI_NMSG];
	int nFrames, event, n=0;
	u_int32 evObj, timeStamp;

	/* Send frames with ID 1..0x14 all received by d2's global fifo */
	BuildMultiFrames( txMsg, 0, TI_NMSG, 1, 0x1 );

	for( n=0; n<TI_NMSG; n++ )
		txMsg[n].id = n+1;

	/* interrupt on last tx complete */
	txMsg[n-1].tx_flags |= ICANL2_FE_GENINT;

	printmsg( 2, "sending frames d1 obj[2]\n");

	CHK( (nFrames = ICANL2API_WriteFifo( d1->path, ICANL2_OOB_HANDLE, txMsg,
										 TI_NMSG )) > 0 );
	CHK2( nFrames == TI_NMSG );

	/* wait for tx complete */
	CHK( (event = ICANL2API_GetEvent( d1->path, 1000, &evObj, &timeStamp))
		 >= 0 );

	printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
			  evObj);

	CHK( event == ICANL2_EV_TX_COMPLETE );

	UOS_Delay(20);

	/* read messages out of global fifo */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[0], rxMsg,TI_NMSG ))
		 >= 0 );
	printmsg( 2, "nFrames=%d\n", nFrames );
	CHK2( nFrames == TI_NMSG );

	if( VerifyMultiFrames( rxMsg, TI_NMSG, 1, 0x1 ) < 0 ) goto abort;
	for( n=0; n<TI_NMSG; n++ )
		CHK2( txMsg[n].id == n+1 );


	/* separate sending of frames */
	printmsg( 2, "sending frames on d1 separately\n");
	for( n=0; n<TI_NMSG; n++ ) {
		txMsg[0].id = n+1;

		printmsg( 3, "frame #%d ", n );
		CHK( (nFrames = ICANL2API_WriteFifo( d1->path, ICANL2_OOB_HANDLE, txMsg,
										 1 )) >= 0 );
		printmsg( 3, " written: %d\n", nFrames );
		CHK2( nFrames == 1 );
	}

	UOS_Delay(20);

	/* read messages out of global fifo */
	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[0], rxMsg,TI_NMSG ))
		 >= 0 );
	printmsg( 2, "nFrames=%d\n", nFrames );
	CHK2( nFrames == TI_NMSG );


	return 0;

 abort:
	return 1;
}

/*Remote transmit frames */
static int TestJ( DEVICE *d1, DEVICE *d2 )
{
	static const u_int8 data[] = { 1,2,3,4,5 };
	ICANL2_DATA req, rsp;
	int event;
	u_int32 evObj, timeStamp;

	/* Post data to RTR object */
	CHK( ICANL2API_WriteObject( d1->path, d1->obj[6], 5, data,
								ICANL2_FE_GENINT ) == 0 );

	/* send the remote transmit request (over OOB object) */
	req.data_len = 0;
	req.tx_flags = ICANL2_FE_REMOTE;
	req.id		 = 0x702;

	CHK( ICANL2API_WriteFifo( d2->path, ICANL2_OOB_HANDLE, &req, 1 ) == 1 );

	/* remote frame must be now received by d2's obj[6] */
	/* wait for siglvl event on d2 */
	CHK( (event = ICANL2API_GetEvent( d2->path, 1000, &evObj, &timeStamp))
		 >= 0 );
	printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
			  evObj);
	CHK2( event == ICANL2_EV_SIGLVL );
	CHK2( evObj == d2->obj[6] );

	CHK( (ICANL2API_ReadFifo( d2->path, d2->obj[6], &rsp, 1 )) == 1);
	DumpFrame( &rsp );

	if( VerifyFrameData( &rsp, 5, 1 ) < 0 )
		goto abort;

	return 0;

 abort:
	return 1;
}

/* Cyclic Tx/Timestamping */
static int TestK( DEVICE *d1, DEVICE *d2 )
{
	#define TK_NOBJ 5
	static const u_int8 data1[] = { 1,2,3,4,5 };
	static const u_int8 data2[] = { 6,7,8 };
	static const u_int8 data3[] = { 9,10,11 };
	ICANL2_DATA rxMsg[1];
	int32 nFrames, rcvd;
	u_int8 startByte;
	u_int32 lastTs, dummy;
	int32 obj[TK_NOBJ];
	int gotData3, size, i;
	double dt;
    u_int8 run;

	/* disable CAN communication */
	CHK( ICANL2API_EnableCan( d1->path, 0 ) == 0 );
	/* create some objects with cyclic sending */
	for( i=0; i<TK_NOBJ; ++i ) {
		CHK( (obj[i] = ICANL2API_CreateObject(
			d1->path,
			0x501+i,				/* id */
			ICANL2_OBJT_STD,	 	/* type */
			ICANL2_MOD_TRANSMIT, 	/* mode */
			0,	 					/* fifo size */
			0						/* signal level */
			)) >= 0 );
	}

	/* enable CAN communication */
	CHK( ICANL2API_EnableCan( d1->path, 1 ) == 0 );

	for( i=0; i<TK_NOBJ; ++i ) {
		CHK( ICANL2API_SendCyclic( d1->path, obj[i], sizeof(data1), data1,
								   3 ) == 0 );
	}

	/*
	 * Internal timer set to 1ms
	 * transmit two cyclic objects:
	 * ID 201: 3ms
	 * ID 301: 1ms
	 */
	CHK( ICANL2API_SetTimer( d1->path, 1000/8 ) == 0 );
	CHK( ICANL2API_SendCyclic( d1->path, d1->obj[1], sizeof(data1), data1,
							   3 ) == 0 );
	CHK( ICANL2API_SendCyclic( d1->path, d1->obj[2], sizeof(data2), data2,
							   1 ) == 0 );

	/* Delete additional objects. */
	CHK( ICANL2API_EnableCan( d1->path, 0 ) == 0 );
	for( i=0; i<TK_NOBJ; ++i ) {
		CHK( ICANL2API_DeleteObject( d1->path, obj[i] ) == 0 );
	}
	CHK( ICANL2API_EnableCan( d1->path, 1 ) == 0 );

	/* Cyclic sending of the remaining objects must work correctly now. */
	CHK( ICANL2API_StartTimer( d1->path, 1 ) == 0);

	UOS_Delay(200);
	/* change data of ID 301 */
	CHK( ICANL2API_SendCyclic( d1->path, d1->obj[2], sizeof(data3), data3,
							   1 ) == 0 );
	UOS_Delay(300);

	/* stop cyclic */
	CHK( ICANL2API_SendCyclic( d1->path, d1->obj[1], 0, NULL, 0 ) == 0 );
	CHK( ICANL2API_SendCyclic( d1->path, d1->obj[2], 0, NULL, 0 ) == 0 );

	UOS_Delay(50);

	/* Verify the received frames (ID 201) must be at least 150 */
	lastTs = 0;
	rcvd = 0;
    run = 1;

	while( run ){
		CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[2], rxMsg, 1 ))
			 >= 0);
		if( nFrames == 0 ) {
            run = 0;
			break;
        }

		rcvd++;
		CHK2( VerifyFrameData( rxMsg, sizeof(data1), data1[0] ) >= 0 );

		/* check timestamp */
		if( lastTs != 0 ){
			dt = (double)(rxMsg[0].time - lastTs) * ICANL2_TCLK;
			printmsg( 3, "%d: dt=%3.2f\n", rcvd, dt );

			if( dt <= 2900 && dt >= 3100 ){
				printmsg(1,"dt=%3.2f last=%x time=%x\n",
						 dt, lastTs, rxMsg[0].time );
				CHK2( 0 );
			}

		}
		lastTs = rxMsg[0].time;
	}
	CHK2( rcvd > 150 );


	/* Verify the received frames (ID 301) must be at least 480 */
	lastTs = 0;
	rcvd = 0;
	gotData3 = 0;
    run = 1;

	while( run ){
		CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[3], rxMsg, 1 ))
			 >= 0);
		if( nFrames == 0 ) {
            run = 0;
			break;
        }

		rcvd++;

		if( rxMsg[0].data[0] == data2[0] ){
			startByte = data2[0];
			size = sizeof(data2);
		}
		else if( rxMsg[0].data[0] == data3[0] ){
			startByte = data3[0];
			size = sizeof(data3);
			gotData3 = 1;
		}
		else {
			CHK2( 0 );
		}

		CHK2( VerifyFrameData( rxMsg, (u_int8)size, startByte ) >= 0 );

		/* check timestamp */
		if( lastTs != 0 ){
			dt = (double)(rxMsg[0].time - lastTs) * ICANL2_TCLK;
			printmsg( 3, "%d: dt=%3.2f\n", rcvd, dt );

			if( dt <= 700 && dt >= 1300 ){
				printmsg(1,"dt=%3.2f last=%x time=%x\n",
						 dt, lastTs, rxMsg[0].time );
				CHK2( 0 );
			}

		}
		lastTs = rxMsg[0].time;
	}
	CHK2( rcvd > 150 );
	CHK2( gotData3 == 1 );

	/* eat the events */
	while( ICANL2API_GetEvent( d2->path, 0, &dummy, &dummy) > 0 )
		;

	return 0;

 abort:
	return 1;
}


static u_int32 G_LastSigCode = 0;

static void __MAPILIB SigHandler( u_int32 sigCode )
{
	printmsg( 3, "Got signal %d\n", sigCode );
	G_LastSigCode = sigCode;
}

/* Signals */
static int TestL( DEVICE *d1, DEVICE *d2 )
{
	#define TL_NMSG 1
	ICANL2_DATA txMsg[TL_NMSG];
	ICANL2_DATA rxMsg[TL_NMSG];

	BuildFrameData( txMsg, 0, 3, 0x11 );
	CHK( UOS_SigInit( SigHandler ) == 0 );

/*dev1:	1		201		Tx		32 		- */
/*dev2:	2		201		Rx		256 	1 */

	/* install signal */
	CHK( UOS_SigInstall( UOS_SIG_USR1 ) == 0 );
	CHK( ICANL2API_Signal( d2->path, UOS_SIG_USR1  ) == 0 );

	printmsg( 3, "Sending signal...\n" );
	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[1], txMsg, TL_NMSG ) > 0 );


	UOS_Delay(20);				/* wait for frame transmit */
	CHK( ICANL2API_ReadFifo( d2->path, d2->obj[2], rxMsg, TL_NMSG ) >= 0 );

	printmsg( 3, "Receive OK.\n" );

	CHK( G_LastSigCode == UOS_SIG_USR1 );
	G_LastSigCode = 0;

	CHK( ICANL2API_Signal( d2->path, 0 ) == 0 );
	CHK( UOS_SigExit() == 0 );

	return 0;

 abort:
	ICANL2API_Signal( d2->path, 0 );
	UOS_SigExit();
	return 1;
}


/* BusOff detection and recovery */
static int TestM( DEVICE *d1, DEVICE *d2 )
{
#define TM_NMSG			5
	ICANL2_DATA txMsg[TM_NMSG], rxMsg[TM_NMSG];
	int event, i, nFrames, ev_found;
	u_int32 evObj, timeStamp;

	BuildMultiFrames( txMsg, 0, TM_NMSG, 8, 0 );

	/* change bus timing */
	CHK( SetTiming( d2, 125000, 0x00 ) == 0 );

	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[1], txMsg, TM_NMSG ) > 0 );

	UOS_Delay(50);				/* wait for frame transmit */

	/* wait for "BusOff" event */
	ev_found = FALSE;
	for( i=0; i<10; ++i ) {
		CHK( (event = ICANL2API_GetEvent( d1->path, 100, &evObj, &timeStamp))
			 >= 0 );

		if( event ) {
			printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
					  evObj);
			if( event == ICANL2_EV_BOFF ) {
				ev_found = TRUE;
				break;
			}

		} else {
			printmsg( 3, "No event\n" );
		}
	}

	CHK2( ev_found );

	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[2], rxMsg, TM_NMSG ))
		 >= 0 );
	printmsg( 3, " got %d frame(s)\n", nFrames );

	/* restore bus timing */
	CHK( SetTiming( d2, G_baud, 0x00 ) == 0 );


	/* initiate BusOff recovery */
	ICANL2API_EnableCan( d1->path, TRUE );


	/* send test messages */
 	CHK( ICANL2API_WriteFifo( d1->path, d1->obj[1], txMsg, TM_NMSG ) > 0 );

	UOS_Delay(50);				/* wait for frame transmit */

	/* wait for "BusOff end" event */
	ev_found = FALSE;
	for( i=0; i<5; ++i ) {
		CHK( (event = ICANL2API_GetEvent( d1->path, 100, &evObj, &timeStamp))
			 >= 0 );

		if( event ) {
			printmsg( 3, "Event: %s obj %d\n", ICANL2API_EventToString(event),
					  evObj);
			if( event == ICANL2_EV_BOFF_END )
				ev_found = TRUE;

		} else {
			printmsg( 3, "No event\n" );
		}
	}

	CHK( (nFrames = ICANL2API_ReadFifo( d2->path, d2->obj[2], rxMsg, TM_NMSG ))
		 == TM_NMSG );
	printmsg( 3, " got %d frame(s)\n", nFrames );
	CHK2( ev_found );

	return 0;

 abort:
	SetTiming( d2, G_baud, 0x00 );
	return 1;
}




