/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: icanl2tbox.h
 *
 *      $Author: ub $
 *        $Date: 2004/04/05 08:59:40 $
 *    $Revision: 2.5 $
 *
 *  Description: Include file for ICANL2 host toolbox library
 *
 *     Switches:
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_tbox.h,v $
 * Revision 2.5  2004/04/05 08:59:40  ub
 * cosmetics
 *
 * Revision 2.4  2002/02/04 15:16:36  ub
 * Added: _ICANL2TBOX_FwRunning(), _ICANL2TBOX_WaitForFw()
 *
 * Revision 2.3  2001/12/20 10:32:40  Schoberl
 * added extern _ICANL2_fw[]
 *
 * Revision 2.2  2001/12/12 14:48:13  ub
 * Prototypes for _ICANL2TBOx_ReadFifo() and _ICANL2TBOX_FifoInfo() changed.
 *
 * Revision 2.1  2001/11/29 11:25:31  ub
 * Initial Revision
 *
 Prototypes changed: ICANL2TBOX_WriteFifoEntry(), ICANL2TBOX_ReadFifoEntry().
 Added: ICANL2TBOX_ReadEvent();
 #defines used in MDIS driver moved to icanl2_codes.h
 #defines for variants added.

 * Revision 2.4  2001/11/21 10:54:49  ub
 * Prototype for ICANL2TBOX_Term() changed.
 *
 * Revision 2.3  2001/11/06 16:46:10  ub
 * All #defines begin with ICANL2_ now.
 *
 * Revision 2.2  2001/10/24 10:19:13  ub
 * Cosmetic changes.
 *
 * Revision 2.1  2001/09/25 14:21:12  ub
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _M65ICANL2_TBOX_H
#define _M65ICANL2_TBOX_H

#ifdef __cplusplus
      extern "C" {
#endif

/* Firmware commands: */

#define ICANL2_CMD_IDENTIFY			0x01
#define ICANL2_CMD_INITCAN			0x02
#define ICANL2_CMD_GETINFO			0x03
#define ICANL2_CMD_SETREG			0x04
#define ICANL2_CMD_GETREG			0x05
#define ICANL2_CMD_GETREGS			0x06
#define ICANL2_CMD_SETACCMASK		0x07
#define ICANL2_CMD_ENABLECAN		0x08
#define ICANL2_CMD_SETTIMER			0x09
#define ICANL2_CMD_STARTTIMER		0x0a
#define ICANL2_CMD_SYNCTIMER		0x0b
#define ICANL2_CMD_CREATEOBJ		0x0c
#define ICANL2_CMD_DELETEOBJ		0x0d
#define ICANL2_CMD_SEND				0x0e
#define ICANL2_CMD_SEND_OOB			0x0f
#define ICANL2_CMD_SEND_CYC			0x10
#define ICANL2_CMD_RCV_EVENT		0x11
#define ICANL2_CMD_RCV_EXTMSG		0x12
#define ICANL2_CMD_RCV_MSG			0x13
#define ICANL2_CMD_NEWESTMSG		0x14
#define ICANL2_CMD_FIFOADR			0x15

/* Flags for ICANL2_STATUS and parameter <status_mask> of HandleIRQ(): */

#define ICANL2_STAT_CMD				0x01			/* Firmware command done. */
#define ICANL2_STAT_EVENT			0x02			/* CAN event. */

/* Firmware error codes: */

#define ICANL2_E_OK					0
#define ICANL2_E_CMD_UNKNOWN		1			/* Host command unknown. */
#define ICANL2_E_FIFO_FULL			2			/* Fifo full. */
#define ICANL2_E_FIFO_EMPTY			3			/* Fifo empty. */
#define ICANL2_E_NO_OBJ				4			/* No free entry left in */
												/*  object table. */
#define ICANL2_E_NO_MEM				5			/* No more memory for object. */
#define ICANL2_E_OBJ_EXISTS			6			/* Object already exists. */
#define ICANL2_E_NOT_FOUND			7			/* Object not found. */
#define ICANL2_E_NO_TRANSMIT		8			/* Object is not configured */
												/*  for transmission. */
#define ICANL2_E_SIGLVL				9			/* Fifo warning level reached.*/
#define ICANL2_E_NODATA				10			/* No new data available.*/
#define ICANL2_E_CYL_FULL			11			/* CycList full.*/
#define ICANL2_E_BUSY				12			/* CAN controller busy. */
#define ICANL2_E_COMM				13			/* Communication error. */
#define ICANL2_E_PARAM				14			/* Parameter out of range. */

/* Flags for <result> in ICANL2_HandleIRQ: */

#define ICANL2_HI_IRQ_RECEIVED		0x01
#define ICANL2_HI_CMD_FINISHED		0x02
#define ICANL2_HI_EVENT_UPDATE		0x04


/* Toolbox error codes: */

#define ICANL2TBOX_ERR_OK			0			/* No error. */
#define ICANL2TBOX_ERR_NOTRDY		0x20		/* Firmware init failed. */
#define ICANL2TBOX_ERR_BUSY			0x21		/* Module busy while new */
												/*  command issued. */
#define ICANL2TBOX_ERR_BADHANDLE	0x22		/* Handle checksum wrong. */
#define ICANL2TBOX_ERR_VERIFY		0x23		/* Firmware verification error*/

#define ICANL2_UNUSED				-1


/* Variants: */

#ifndef ICANL2_VARIANT
# define ICANL2_VARIANT ICANL2
#endif

#define _ICANL2TBOX_GLOBNAME(var,name) var##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
# define ICANL2TBOX_GLOBNAME(var,name) _ICANL2TBOX_GLOBNAME(var,name)
#else
# define ICANL2TBOX_GLOBNAME(var,name) _ICANL2TBOX_GLOBNAME(ICANL2,name)
#endif

/* variant specific function names */
#define _ICANL2TBOX_HandleSize			ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_HandleSize)
#define _ICANL2TBOX_Init		        ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_Init)
#define _ICANL2TBOX_Term		        ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_Term)
#define _ICANL2TBOX_InitCom             ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_InitCom)
#define _ICANL2TBOX_Command             ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_Command)
#define _ICANL2TBOX_FwRunning           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_FwRunning)
#define _ICANL2TBOX_WaitForFw           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_WaitForFw)
#define _ICANL2TBOX_HandleIRQ           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_HandleIRQ)
#define _ICANL2TBOX_IRQEnable           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_IRQEnable)
#define _ICANL2TBOX_Reset               ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_Reset)
#define _ICANL2TBOX_ReadFifo            ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_ReadFifo)
#define _ICANL2TBOX_ReadCommArea        ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_ReadCommArea)
#define _ICANL2TBOX_WriteFifoEntry      ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_WriteFifoEntry)
#define _ICANL2TBOX_ReadFifoEntry       ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_ReadFifoEntry)
#define _ICANL2TBOX_NewestMsg           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_NewestMsg)
#define _ICANL2TBOX_ReadEvent           ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_ReadEvent)
#define _ICANL2TBOX_FifoInfo            ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,TBOX_FifoInfo)

#define _ICANL2_fw						ICANL2TBOX_GLOBNAME(ICANL2_VARIANT,_fw)

/*-----------------------------------------+
|  EXTERNALS                               |
+-----------------------------------------*/
extern const u_int8 _ICANL2_fw[];

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
typedef int8 boolean;

/* Structure for a single CAN controller. */
typedef struct {
	MACCESS		ma;						/* Access descriptor. */
	u_int8		*reqData;				/* Request data. */
	u_int8		*rspData;				/* Response data. */
	u_int16		reqLen;					/* Length of request data. */
	u_int16		rspLen;					/* Length of response data. */
	u_int8		opcode;					/* Command opcode. */
	u_int8		winadrMsw;				/* MSW of window adr reg. (offset) */
	u_int8		winadrLsw;				/* LSW of window adr reg. (offset) */
	u_int8		ptradrMsw;				/* MSW of pointer adr reg. (offset) */
	u_int8		ptradrLsw;				/* LSW of pointer adr reg. (offset) */

} ICANL2_HANDLE;

/* Structure for complete M65 module. */
typedef struct {
	u_int32				magic;			/* Handle magic word. */
	MACCESS				ma;				/* Acc. descriptor for entire module. */
	ICANL2_HANDLE		can[2];			/* Storage for CAN controllers. */
	u_int16				flags;			/* Global flags. */

} ICANL2TBOX_HANDLE;


/* Structure for host communication. */
/* ATTENTION: <HOST_PARAM> must be identical with the corresponding struct */
/* in the firmware. */
typedef struct {
	int32		p[6];
	u_int8		data[8];
} ICANL2_HOST_PARAM;


/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/

int32 _ICANL2TBOX_HandleSize(void);

int32 _ICANL2TBOX_Init(
	ICANL2TBOX_HANDLE *h,
	MACCESS ma );

int32 _ICANL2TBOX_Term(
	ICANL2TBOX_HANDLE *h,
    u_int8     canNum );

int32 _ICANL2TBOX_InitCom(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum );

int32 _ICANL2TBOX_FwRunning( ICANL2TBOX_HANDLE *h, u_int8 canNum );

int32 _ICANL2TBOX_WaitForFw( ICANL2TBOX_HANDLE *h, u_int8 canNum , int32 ms );

int32 _ICANL2TBOX_Command(
	ICANL2TBOX_HANDLE  *h,
    u_int8     canNum,
    u_int8     opcode,
    u_int8     *reqData,
    int16       reqLen,
    u_int8     *rspData,
    int16       rspLen );

int32 _ICANL2TBOX_HandleIRQ(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum,
    u_int8     status_mask,
    u_int8     *resultP,
    u_int8     *fwResultP );

int32 _ICANL2TBOX_IRQEnable(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum,
    u_int8     enable );

int32 _ICANL2TBOX_Reset(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum );

int32 _ICANL2TBOX_ReadFifo(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int16 handle,
	u_int16 *objCnt,
	u_int8 *data );

void _ICANL2TBOX_ReadCommArea(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int8 *dst );

int32 _ICANL2TBOX_WriteFifoEntry(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int16 handle,
	ICANL2_DATA *data,
	u_int32 *numEntries );

int32 _ICANL2TBOX_ReadFifoEntry(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int16 handle,
	ICANL2_DATA *data,
	u_int32 *numEntries );

int32 _ICANL2TBOX_NewestMsg(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int16 handle,
	u_int8 *data_len_p,
	u_int8 *data_p,
	u_int32 *id_p,
	u_int32 *time_p  );

int32 _ICANL2TBOX_ReadEvent(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	ICANL2_EVENT *ev,
	u_int32 *numEntries );

int32 _ICANL2TBOX_FifoInfo(
	ICANL2TBOX_HANDLE *h,
	u_int8 canNum,
	u_int16 handle,
	int32 *numEntries );

#ifdef __cplusplus
      }
#endif

#endif /* _M65ICANL2_TBOX_H */




