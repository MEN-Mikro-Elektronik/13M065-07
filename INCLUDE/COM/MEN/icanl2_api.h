/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: icanl2_api.h
 *
 *       Author: kp
 *        $Date: 2009/06/30 11:48:37 $
 *    $Revision: 2.4 $
 *
 *  Description: Header file for ICANL2 (intelligent CAN layer 2) API
 *
 *     Switches:
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_api.h,v $
 * Revision 2.4  2009/06/30 11:48:37  CRuff
 * R: 1.Porting to MDIS5
 * M: 1.changed according to MDIS Porting Guide 0.5
 *
 * Revision 2.3  2002/02/04 15:16:42  ub
 * Added: ICANL2API_FwIdent(), ICANL2API_FwInfo(), ICANL2API_Signal()
 * Deleted: ICANL2API_InitCan()
 *
 * Revision 2.2  2001/12/12 14:48:17  ub
 * Added: ICANL2API_FifoInfo()
 *
 * Revision 2.1  2001/11/29 12:00:28  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _ICANL2_API_H
#define _ICANL2_API_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/* none */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/

/* all API functions return >=0 on success and -1 on error (errno set) */

int32 __MAPILIB ICANL2API_SetTiming(
	MDIS_PATH path,
	u_int8 brp,
	u_int8 sjw,
	u_int8 tseg1,
	u_int8 tseg2,
	u_int8 spl,
	u_int8 busconf );

int32 __MAPILIB ICANL2API_SetAccMask(
	MDIS_PATH path,
	u_int32 accMask,
	u_int8 mode);

int32 __MAPILIB ICANL2API_CreateObject(
	MDIS_PATH path,
	u_int32 id,
	u_int32 type,
	u_int32 tMode,
	u_int32 fifoSize,
	u_int32 sigLvl);

int32 __MAPILIB ICANL2API_DeleteObject(
	MDIS_PATH path,
	u_int32 objHdl);

int32 __MAPILIB ICANL2API_EnableCan(
	MDIS_PATH path,
	u_int8 enable);

int32 __MAPILIB ICANL2API_ReadFifo(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf,
	u_int32 maxFrames);

int32 __MAPILIB ICANL2API_WriteFifo(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf,
	u_int32 nFrames);

int32 __MAPILIB ICANL2API_NewestMsg(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf);

int32 __MAPILIB ICANL2API_FifoInfo(
	MDIS_PATH path,
	u_int32 objHdl,
	int32 *numEntries );

int32 __MAPILIB ICANL2API_FwIdent(
	MDIS_PATH path,
	int32 *ver );

int32 __MAPILIB ICANL2API_FwInfo(
	MDIS_PATH path,
	int32 *numEvents,
	int32 *tick,
	int32 *freeMem,
	int32 *numCyc );

int32 __MAPILIB ICANL2API_GetEvent(
	MDIS_PATH path,
	int32 timeout,
	u_int32 *objHdlP,
	u_int32 *timeStampP);

int32 __MAPILIB ICANL2API_Signal(
	MDIS_PATH path,
	int32 signal);

int32 __MAPILIB ICANL2API_WriteObject(
	MDIS_PATH path,
	u_int32 objHdl,
	u_int8	dataLen,
	const u_int8  *data,
	u_int16	flags);

int32 __MAPILIB ICANL2API_SendCyclic(
	MDIS_PATH path,
	u_int32 objHdl,
	u_int8	dataLen,
	const u_int8  *data,
	u_int32	cycleTime);

int32 __MAPILIB ICANL2API_SetTimer(
	MDIS_PATH path,
	u_int32 tickIv);

int32 __MAPILIB ICANL2API_StartTimer(
	MDIS_PATH path,
	int32 on);

int32 __MAPILIB ICANL2API_SyncTimer(
	MDIS_PATH path);

const char * __MAPILIB ICANL2API_EventToString(
	int32 evCode);

char * __MAPILIB ICANL2API_ErrorToString(
	int32 error);


#ifdef __cplusplus
      }
#endif

#endif /* _ICANL2_DRV_H */



