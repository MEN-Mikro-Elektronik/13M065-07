/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2_api.c
 *      Project: M65/P5 intelligent layer 2 driver
 *
 *       Author: kp
 *        $Date: 2010/03/09 16:48:53 $
 *    $Revision: 1.4 $
 *
 *  Description: Convert ICANL2 API codes to strings
 *
 *     Required: ICANL2 MDIS4 LL driver
 *     Switches: -
 *
 *---------------------------[ Public Functions ]----------------------------
 *
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_strings.c,v $
 * Revision 1.4  2010/03/09 16:48:53  amorbach
 * R: Compiler warnings
 * M: Fixed compiler warings
 *
 * Revision 1.3  2002/02/04 15:16:29  ub
 * Added: strings for <EV_BOFF_END> and <EV_WARN_END>.
 *
 * Revision 1.2  2001/12/12 14:48:11  ub
 * Event strings fixed.
 *
 * Revision 1.1  2001/11/29 12:00:24  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include <MEN/men_typs.h>
#include <MEN/mdis_err.h>
#include <MEN/mdis_api.h>
#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_drv.h>
#include <MEN/icanl2_api.h>
#include <stdio.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
/* none */

/********************************* ICANL2API_EventToString ********************
 *
 *  Description: Convert event to string
 *
 *---------------------------------------------------------------------------
 *  Input......: evCode     event code
 *  Output.....: returns:   ptr to
 *                          -1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
const char * __MAPILIB ICANL2API_EventToString(
    int32 evCode)
{
    static const char* evstr[] = {
    "EV_NONE",
    "EV_SIGLVL",                            /* Nbr. fifo entries exceeded */
                                            /*  warning level. */
    "EV_BOFF",                              /* i82527 in BusOff status */
    "EV_MSG_LOST",                          /* Message lost. */
    "EV_WARN",                              /* i82527 warning */
    "EV_FIFO_FULL",                         /* Fifo full. */
    "EV_TXACK_FULL",                        /* TxAck fifo full. */
    "EV_TX_COMPLETE",                       /* Transmission complete. */
    "EV_BOFF_END",                          /* i82527 leaves BusOff status. */
    "EV_WARN_END",                          /* i82527 leaves Warning status. */

    "?", "?", "?",
    "?", "?", "?",  "?",                    /* Unknown events */
    "EV_STUFF_ERROR",                       /* i82527 stuff error */
    "EV_FORM_ERROR",                        /* i82527 msg malformed */
    "EV_ACK_ERROR",                         /* i82527 msg not acknowledged*/
    "EV_BIT1_ERROR",                        /* i82527 CAN bus stuck to 1 */
    "EV_BIT0_ERROR",                        /* i82527 CAN bus stuck to 0 */
    "EV_CRC_ERROR"                          /* i82527 msg wrong crc */
    };


    if( evCode < sizeof(evstr)/sizeof(char *))
        return evstr[evCode];
    else
        return "?";
}

/********************************* ICANL2API_ErrorToString ********************
 *
 *  Description: Convert error number to string
 *
 *  This routine is non-reentrant since it uses a local static buffer
 *---------------------------------------------------------------------------
 *  Input......: error      error code
 *  Output.....: returns:   ptr to
 *                          -1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
char * __MAPILIB ICANL2API_ErrorToString(
    int32 error)
{
    static char errMsg[128];
    static const char* errstr[] = {
        "fifo full",
        "fifo empty",
        "no more objects",
        "no memory",
        "object exists",
        "object not found",
        "fifo warning level reached", /* ??? */
        "no new data available",
        "cyclic list full",
        "verify error",
        "no interrupt"          /* ??? */
    };

    if( error >= ICANL2_ERR_FIFOFULL && error <= ICANL2_ERR_NOIRQ){
        sprintf( errMsg, "ERROR (ICANL2) %d:  %s", (int)error,
                 errstr[error-(ICANL2_ERR_FIFOFULL)]);
        return errMsg;
    }
    else
        return M_errstring(error);
}




