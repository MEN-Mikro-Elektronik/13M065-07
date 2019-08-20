/****************************************************************************
 ************                                                    ************
 ************                   ICANL2_SIGNAL                    ************
 ************                                                    ************
 ****************************************************************************
 *
 *       Author: ub
 *        $Date: 2010/03/09 16:48:51 $
 *    $Revision: 1.4 $
 *
 *  Description: Simple example program for using the ICANL2 driver with signals
 *
 *               - Configures CAN chip for 1MBps, basic (11 bit) CAN mode
 *               - Creates a general message fifo to store all incoming
 *                 CAN messages
 *               - Creates one Tx object with ID 0x123 and sends a single
 *                 frame with that ID
 *               - Installs signal handler and configures driver to generate
 *                 signal at every event.
 *               - Waits for events on CAN bus. Any incoming event and CAN
 *                 message is printed to stdout
 *               - Exits when more than 100 events received
 *
 *     Required: libraries: mdis_api, icanl2_api, usr_oss
 *     Switches: -
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2002-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/

 /*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

static const char RCSid[]="$Id: icanl2_signal.c,v 1.4 2010/03/09 16:48:51 amorbach Exp $";

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

/*--------------------------------------+
|  PROTOTYPES                           |
+--------------------------------------*/
static void __MAPILIB SigHandler( u_int32 sigCode );

/*--------------------------------------+
|  GLOBALS                              |
+--------------------------------------*/
static int G_signal;


/********************************* main *************************************
 *
 *  Description: Program main function
 *
 *---------------------------------------------------------------------------
 *  Input......: argc,argv  argument counter, data ..
 *  Output.....: return     success (0) or error (1)
 *  Globals....: -
 ****************************************************************************/
int main(int argc, char *argv[])
{
    MDIS_PATH   path=-1;
    char    *device;
    int32   rxObj, txObj, eventCount=0;
    int32   ver;
    u_int8  run;

    if (argc < 2 || strcmp(argv[1],"-?")==0) {
        printf("Syntax: icanl2_signal <device> <chan>\n");
        printf("Function: ICANL2 simple example for M065/P5\n");
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
                              0,            /* brp */
                              2,            /* sjw */
                              3,            /* tseg1 */
                              2,            /* tseg2 */
                              1,            /* spl */
                              0x00          /* BusConf */
        ) == 0);


    /* set acceptence mask and operation mode */
    CHK( ICANL2API_SetAccMask( path, 0x7FF, 0 ) == 0 );/* basic (11bit) mode */

    /* get and print some information from firmware */
    CHK( ICANL2API_FwIdent( path, &ver ) == 0 );
    printf( "Firmware revision: %d.%d.%ld\n", (int)ver>>16, (int)(ver&0xff00)>>8,
                                            ver&0xff );
    /* create general message fifo (stores ALL incoming messages) */
    printf("Create objects\n");
    CHK( (rxObj = ICANL2API_CreateObject( path,
                                          0,           /* id is ignored here */
                                          ICANL2_OBJT_GENERAL,  /* type */
                                          ICANL2_MOD_RECEIVE,   /* mode */
                                          8192,                 /* fifo size */
                                          1                 /* signal level */
        )) >= 0 );

    /* create a Tx object */
    CHK( (txObj = ICANL2API_CreateObject( path,
                                          0x123,                /* id */
                                          ICANL2_OBJT_STD,      /* type */
                                          ICANL2_MOD_TRANSMIT,  /* mode */
                                          32,               /* fifo size */
                                          0                 /* don't care */
        )) >= 0 );

    /* enable can communication */
    printf("Enable can communication\n");
    CHK( ICANL2API_EnableCan( path, 1 ) == 0 );


    /* send a message to our tx object */
    {
        ICANL2_DATA msg;

        msg.data_len    = 6;
        msg.tx_flags    = ICANL2_FE_GENINT; /* generate event on Tx complete */
        msg.data[0]     = 'H';
        msg.data[1]     = 'E';
        msg.data[2]     = 'L';
        msg.data[3]     = 'L';
        msg.data[4]     = 'O';
        msg.data[5]     = '\0';

        CHK( ICANL2API_WriteFifo( path, txObj, &msg, 1 ) > 0 );
    }

    /* install signal */
    CHK( UOS_SigInit( SigHandler ) == 0 );
    CHK( UOS_SigInstall( UOS_SIG_USR1 ) == 0 );
    CHK( ICANL2API_Signal( path, UOS_SIG_USR1  ) == 0 );

    /* wait for events */
    while( eventCount++ < 100 ) {
        int32 event;
        u_int32 obj, timeStamp;

        /* wait for signal */
        while( G_signal == 0 )
            ;

        /* break, if unknown */
        if( G_signal == -1 )
            break;

        G_signal = 0;

        CHK( (event = ICANL2API_GetEvent( path, 0, &obj, &timeStamp)) >= 0 );

        if( event == 0 )
            printf("No events...\n");
        else {
            printf("event 0x%x (%s) object %d time=%10lu us\n",
                   (int)event, ICANL2API_EventToString(event), (int)obj, timeStamp * 5 );

            if( (event == ICANL2_EV_SIGLVL) && (obj==rxObj) ){
                ICANL2_DATA msg[10];
                int32 nFrames, i, j;

                /* read from general message fifo */
                run = 1;
                while( run ) {
                    /* read from general message fifo */
                    CHK( (nFrames = ICANL2API_ReadFifo( path, obj, msg,
                                                        10 )) >= 0 );

                    if( nFrames == 0 ) {
                        run = 0;
                        break;
                    }

                    for( i=0; i<nFrames; i++ ){
                        printf("obj=%d Id=0x%08x time=%10lu us Data=",
                               (int)obj, (int)msg[i].id,
                               msg[i].time * 5);

                        for( j=0; j<msg[i].data_len; j++ )
                            printf("%02x ", msg[i].data[j] );

                        printf("\n");
                    }
                }
            }
        }
    }

    /* disable can communication */
    printf("Disable can communication\n");
    CHK( ICANL2API_EnableCan( path, 0 ) == 0 );

 abort:
    /* remove signals */
    CHK( UOS_SigExit() == 0 );

    if( path >= 0 )
        M_close(path);

    return(0);
}

/* signal handler routine */
static void __MAPILIB SigHandler( u_int32 sigCode )
{
    /* received our signal ? */
    if( sigCode == UOS_SIG_USR1 )
        G_signal = 1;

    else {
        printf( "Unknown signal received: sigCode = %x\n", (unsigned int)sigCode );
        G_signal = -1;
    }
}



