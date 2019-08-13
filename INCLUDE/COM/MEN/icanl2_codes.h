/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: icanl2_codes.h
 *
 *      $Author: ub $
 *        $Date: 2002/02/04 15:16:38 $
 *    $Revision: 2.3 $
 *
 *  Description: Include file for ICANL2 host toolbox library
 *
 *     Switches:
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_codes.h,v $
 * Revision 2.3  2002/02/04 15:16:38  ub
 * Added: #defines for leaving i82527 BusOff and Warning Status
 *
 * Revision 2.2  2001/11/29 17:49:02  ub
 * Comments added.
 *
 * Revision 2.1  2001/11/29 11:25:11  ub
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _ICANL2_CODES_H
#define _ICANL2_CODES_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/

/* Firmware events: */

#define ICANL2_EV_NONE				0x00
#define ICANL2_EV_SIGLVL			0x01		/* Nbr. fifo entries exceeded */
												/*  warning level. */
#define ICANL2_EV_BOFF				0x02		/* i82527 in BusOff status */
#define ICANL2_EV_MSG_LOST			0x03		/* Message lost. */
#define ICANL2_EV_WARN				0x04		/* i82527 warning */
#define ICANL2_EV_FIFO_FULL			0x05		/* Fifo full. */
#define ICANL2_EV_TXACK_FULL		0x06		/* TxAck fifo full. */
#define ICANL2_EV_TX_COMPLETE		0x07		/* Transmission complete. */
#define ICANL2_EV_BOFF_END			0x08		/* i82527 leaves BusOff status*/
#define ICANL2_EV_WARN_END			0x09		/* i82527 leaves Warn status. */

#define ICANL2_EV_HWERROR_START		0x10		/* Start of hw error definitions */
#define ICANL2_EV_STUFF_ERROR		0x11		/* i82527 stuff error */
#define ICANL2_EV_FORM_ERROR		0x12		/* i82527 msg malformed */
#define ICANL2_EV_ACK_ERROR			0x13		/* i82527 msg not acknowledged*/
#define ICANL2_EV_BIT1_ERROR		0x14		/* i82527 CAN bus stuck to 1 */
#define ICANL2_EV_BIT0_ERROR		0x15		/* i82527 CANbus stuck to 0 */
#define ICANL2_EV_CRC_ERROR			0x16		/* i82527 msg wrong crc */
#define ICANL2_EV_WAKE				0x20		/* i82527 wake up */

/* General #defines: */

#define ICANL2_MAX_RTR_OBJECTS		10			/* Max. nbr. of RTR objects. */
#define ICANL2_MAX_MSG_OBJECTS		2048		/* Max. nbr. std msg objects. */

/* #defines for handles: */

#define ICANL2_EVENT_HANDLE			0
#define ICANL2_GLOBAL_HANDLE		1
#define ICANL2_GENERAL_HANDLE		2
#define ICANL2_OOB_HANDLE			3
#define ICANL2_LASTMSG_HANDLE		4
#define ICANL2_FIRST_RTR_HANDLE		10
#define ICANL2_FIRST_STD_HANDLE		(ICANL2_FIRST_RTR_HANDLE +  \
									ICANL2_MAX_RTR_OBJECTS)
#define ICANL2_MAX_HANDLE			(ICANL2_FIRST_STD_HANDLE + \
									ICANL2_MAX_MSG_OBJECTS)

/* #defines for <ICANL2_MSG_OBJECT.type>: */
/* ATTENTION: These must be consistent with the firmware #defines. */
#define ICANL2_OBJT_STD				0			/* Standard message object. */
#define ICANL2_OBJT_RTR				1			/* RTR object. */
#define ICANL2_OBJT_GLOBAL			2			/* Global message object. */
#define ICANL2_OBJT_GENERAL			3			/* General message object. */
#define ICANL2_OBJT_EVENT			4			/* Event. */

/* #defines for <ICANL2_STD_MSG_FIFO_ENTRY.flags>: */
/* ATTENTION: These must be consistent with the firmware #defines. */
#define ICANL2_FE_GENINT			0x4000		/* Generate interrupt after */
												/*  transmission. */
#define ICANL2_FE_REMOTE			0x8000		/* Send remote request frame. */

/* #defines for <ICANL2_MSG_OBJECT.tmode>: */
/* ATTENTION: These must be consistent with the firmware #defines. */
#define ICANL2_MOD_RECEIVE 			0			/* Object is receiver */
#define ICANL2_MOD_TRANSMIT			1           /* Object is transmitter */
#define ICANL2_MOD_REMOTE			2           /* Sends remote frames only. */

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/

/* Structure for passing data to/from ICANL2TBOX_Write/ReadFifoEntry(): */
typedef struct {
	u_int8		data_len;		/* Length of CAN data to be sended/received.*/
	u_int8		pad;
	u_int16		tx_flags;		/* Transmit flags.                        */
	u_int8		data[8];        /* CAN data.                              */
	u_int32		id;             /* CAN id (used for Rx and OOB objects)   */
	u_int32		time;			/* Time code of receival (Rx objects only)*/
} ICANL2_DATA;

/* Structure for returning events from ICANL2TBOX_ReadEvent(): */
typedef struct {
	u_int32		timeStamp;		/* Time, when event happened. */
	u_int16		objHdl;			/* Handle of object that caused event. */
								/* (Not supported with all events) */
	u_int8		event;			/* Event code */
	u_int8		used;			/* For internal use. */
} ICANL2_EVENT;


#ifdef __cplusplus
      }
#endif

#endif /* _ICANL2_CODES_H */


