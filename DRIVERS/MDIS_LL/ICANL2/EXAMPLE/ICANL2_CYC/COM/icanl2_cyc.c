/****************************************************************************
 ************                                                    ************
 ************                   ICANL2_CYC                       ************
 ************                                                    ************
 ****************************************************************************
 *
 *       Author: kp
 *        $Date: 2009/06/29 15:52:10 $
 *    $Revision: 1.2 $
 *
 *  Description: Example program for the ICANL2 driver: cyclic transmissions
 *
 *               - Configures CAN chip for 1MBps, basic (11 bit) CAN mode
 *				 - Creates some Tx objects
 *				 - Start to transmit these Tx objects cyclically
 *				 - Waits 5s
 *				 - disables CAN
 *
 *     Required: libraries: mdis_api, icanl2_api, usr_oss
 *     Switches: -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_cyc.c,v $
 * Revision 1.2  2009/06/29 15:52:10  CRuff
 * R: 1. new type MDIS_PATH
 * M: 1. changed type of variable path to MDIS_PATH
 *
 * Revision 1.1  2001/12/12 16:26:50  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: icanl2_cyc.c,v 1.2 2009/06/29 15:52:10 CRuff Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>

#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_drv.h>
#include <MEN/icanl2_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define CHK(expression) \
 if( !(expression)) {\
	 printf("*** Error during: %s\nfile %s\nline %d\n", \
      #expression,__FILE__,__LINE__);\
      printf("%s\n",ICANL2API_ErrorToString(UOS_ErrnoGet()));\
     goto abort;\
 }

/********************************* main *************************************
 *
 *  Description: Program main function
 *
 *---------------------------------------------------------------------------
 *  Input......: argc,argv	argument counter, data ..
 *  Output.....: return	    success (0) or error (1)
 *  Globals....: -
 ****************************************************************************/
int main(int argc, char *argv[])
{
	MDIS_PATH	path=-1;
	char	*device;
	int32 	txObj[5], i;
	static const struct {
		u_int32 id;
		u_int8 mode;
		u_int32 cycTime;
		u_int8 dataLen;
		u_int8 data[8];
	} objDescr[] = {	
		{ 0x101, ICANL2_MOD_TRANSMIT, 1, 5, { 1,2,3,4,5 } },
		{ 0x102, ICANL2_MOD_TRANSMIT, 2, 3, { 6,7,8 } },
		{ 0x103, ICANL2_MOD_REMOTE,   3, 0, { 0 } },
		{ 0x104, ICANL2_MOD_TRANSMIT, 4, 4, { 0xaa,0xbb,0xcc,0xdd } },
		{ 0x105, ICANL2_MOD_TRANSMIT, 5, 8, { 8,7,6,5,4,3,2,1 }}
	};

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: icanl2_cyc <device> <chan>\n");
		printf("Function: ICANL2 cycle timer example for M065/P5\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("\n");
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	CHK((path = M_open(device)) >= 0);

	/*--------------------+
    |  config             |
    +--------------------*/

	/* Timing initialization. Values select 1Mbps bit rate. */
	printf("Configure bus parameters\n");
	CHK( ICANL2API_SetTiming( path, 
							  0,			/* brp */
							  2,			/* sjw */
							  3,			/* tseg1 */
							  2,			/* tseg2 */
							  1,			/* spl */
							  0x00 			/* BusConf */
		) == 0);	
	

	/* set acceptence mask and operation mode */
	CHK( ICANL2API_SetAccMask( path, 0x7FF, 0 ) == 0 );/* basic (11bit) mode */


	/* create Tx objects */
	for(i=0; i<5; i++ ){
		CHK( (txObj[i] = ICANL2API_CreateObject( 
			path,
			objDescr[i].id,		/* id */
			ICANL2_OBJT_STD,	/* type */
			objDescr[i].mode, 	/* mode */
			1, 					/* fifo size */
			0  				  	/* don't care */
		)) >= 0 );
	}

	printf("Enable can communication\n");
	CHK( ICANL2API_EnableCan( path, 1 ) == 0 );

	/* 
	 * Internal timer set to 1ms
	 */
	
	for(i=0; i<5; i++ ){
		CHK( ICANL2API_SendCyclic( 
			path, 
			txObj[i], 
			objDescr[i].dataLen, 
			objDescr[i].data,
			objDescr[i].cycTime ) == 0 );
	}
		
	/* start timer */
	CHK( ICANL2API_SetTimer( path, 1000/8 ) == 0 );	/* set to 1ms */
	CHK( ICANL2API_StartTimer( path, 1 ) == 0);

	printf("Send cyclic objects for 5 seconds\n");
	UOS_Delay(5000);

	/* stop cycle timer */
	CHK( ICANL2API_StartTimer( path, 0 ) == 0);


	/* disable can communication */
	printf("Disable can communication\n");
	CHK( ICANL2API_EnableCan( path, 0 ) == 0 );

 abort:
	getchar();
	if( path >= 0 )
		M_close(path);
			
	return(0);
}

