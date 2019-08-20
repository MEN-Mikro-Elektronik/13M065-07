/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2_api.c
 *      Project: M65/P5 intelligent layer 2 driver
 *
 *       Author: kp
 *
 *  Description: Convert ICANL2 API codes to strings
 *
 *     Required: ICANL2 MDIS4 LL driver
 *     Switches: -
 *
 *---------------------------[ Public Functions ]----------------------------
 *
 *
 *
 *---------------------------------------------------------------------------
 * Copyright 2001-2019, MEN Mikro Elektronik GmbH
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




