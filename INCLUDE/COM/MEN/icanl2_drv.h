/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: icanl2_drv.h
 *
 *       Author: ub
 *        $Date: 2009/06/30 11:49:08 $
 *    $Revision: 2.5 $
 *
 *  Description: Header file for ICANL2 driver
 *               - ICANL2 specific status codes
 *               - ICANL2 function prototypes
 *
 *     Switches: _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_drv.h,v $
 * Revision 2.5  2009/06/30 11:49:08  CRuff
 * R: 1.Porting to MDIS5
 * M: 1.changed according to MDIS Porting Guide 0.5
 *
 * Revision 2.4  2004/04/05 08:59:42  ub
 * cosmetics
 *
 * Revision 2.3  2002/02/04 15:16:40  ub
 * Added: ICANL2_FWINFO_PB
 *
 * Revision 2.2  2001/12/12 14:48:15  ub
 * Added: ICANL2_FIFOINFO_PB.
 *
 * Revision 2.1  2001/11/29 12:00:27  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _ICANL2_DRV_H
#define _ICANL2_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/

typedef struct {
	u_int8 brp;
	u_int8 sjw;
	u_int8 tseg1;
	u_int8 tseg2;
	u_int8 spl;
	u_int8 busconf;
} ICANL2_SETTIMING_PB;

typedef struct {
	u_int32 accMask;
	u_int8 mode;
} ICANL2_SETACCMASK_PB;

typedef struct {
	u_int32 id;
	u_int32 type;
	u_int32 tMode;
	u_int32 fifoSize;
	u_int32 sigLvl;
	u_int32	objHdl;				/* out */
} ICANL2_CREATEOBJECT_PB;

typedef struct {
	u_int32 objHdl;
	ICANL2_DATA msg;			/* out */
	int result;					/* out: 0=ok 1=no data */
} ICANL2_NEWESTMSG_PB;

typedef struct {
	u_int32 objHdl;
	int32 numEntries;			/* out: number of entries used */
} ICANL2_FIFOINFO_PB;

typedef struct {
	int32 numEvents;			/* out: number of entries in event fifo */
	int32 tick;					/* out: tick counter */
	int32 freeMem;				/* out: remaining memory (bytes) */
	int32 numCyc;				/* out: number of cyclic objects */
} ICANL2_FWINFO_PB;

typedef struct {
	int32 timeout;
	u_int32	event;				/* out */
	u_int32 objHdl;				/* out */
	u_int32 timeStamp;			/* out */
} ICANL2_GETEVENT_PB;

typedef struct {
	u_int32 objHdl;
	u_int8 data[8];
	u_int16 flags;
	u_int8 dataLen;
} ICANL2_WRITEOBJECT_PB;

typedef struct {
	u_int32 objHdl;
	u_int8 data[8];
	u_int32 cycleTime;
	u_int8 dataLen;
} ICANL2_SENDCYCLIC_PB;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* ICANL2 specific status codes (STD) */		/* S,G: S=setstat, G=getstat */
#define ICANL2_DELETEOBJECT 	M_DEV_OF+0x00   /*   S: delete msg object  */
#define ICANL2_ENABLECAN        M_DEV_OF+0x01	/*   S: enable/disable CAN   */
#define ICANL2_INITCAN        	M_DEV_OF+0x02	/*   S: initialize CAN       */
#define ICANL2_FWIDENT          M_DEV_OF+0x03	/* G  : firmware version     */
#define ICANL2_SETTIMER     	M_DEV_OF+0x04	/*   S: set timer tick       */
#define ICANL2_STARTTIMER     	M_DEV_OF+0x05	/*   S: start timer          */
#define ICANL2_SYNCTIMER     	M_DEV_OF+0x06	/*   S: syncronize timer     */
#define ICANL2_SETSIG_EVENT		M_DEV_OF+0x10 	/*   S: install event sig    */
#define ICANL2_CLRSIG_EVENT		M_DEV_OF+0x11 	/*   S: deinstall event sig  */

/* icanl2 specific status codes (BLK)	*/		/* S,G: S=setstat, G=getstat */
#define ICANL2_BLK_SETTIMING    M_DEV_BLK_OF+0x00 /*   S: CAN bus timing     */
#define ICANL2_BLK_SETACCMASK  	M_DEV_BLK_OF+0x01 /*   S: set acceptance mask*/
#define ICANL2_BLK_CREATEOBJECT M_DEV_BLK_OF+0x02 /* G  : create msg object  */
#define ICANL2_BLK_NEWESTMSG	M_DEV_BLK_OF+0x03 /* G  : get recent message */
#define ICANL2_BLK_GETEVENT		M_DEV_BLK_OF+0x04 /* G  : get event          */
#define ICANL2_BLK_WRITEOBJECT  M_DEV_BLK_OF+0x05 /*   S: write object data  */
#define ICANL2_BLK_SENDCYCLIC   M_DEV_BLK_OF+0x06 /*   S: write cyclic data  */
#define ICANL2_BLK_FWINFO		M_DEV_BLK_OF+0x07 /* G  : get firmware info  */
#define ICANL2_BLK_FIFOINFO		M_DEV_BLK_OF+0x08 /* G  : get fifo info  */

/* ICANL2 specific error codes */
#define ICANL2_ERR_FIFOFULL     (ERR_DEV+0x1)
#define ICANL2_ERR_FIFOEMPTY    (ERR_DEV+0x2)
#define ICANL2_ERR_NOOBJ        (ERR_DEV+0x3)
#define ICANL2_ERR_NOMEM        (ERR_DEV+0x4)
#define ICANL2_ERR_OBJEXISTS    (ERR_DEV+0x5)
#define ICANL2_ERR_NOTFOUND     (ERR_DEV+0x6)
#define ICANL2_ERR_SIGLVL       (ERR_DEV+0x7)
#define ICANL2_ERR_NODATA       (ERR_DEV+0x8)
#define ICANL2_ERR_CYL_FULL     (ERR_DEV+0x9)
#define ICANL2_ERR_VERIFY       (ERR_DEV+0xa)
#define ICANL2_ERR_NOIRQ        (ERR_DEV+0xb)

#define ICANL2_ERR_UNK_ERR      (ERR_DEV+0xf)


/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/

#ifndef  ICANL2_VARIANT
# define ICANL2_VARIANT ICANL2
#endif

# define _ICANL2_GLOBNAME(var,name) var##_##name
#ifndef _ONE_NAMESPACE_PER_DRIVER_
# define ICANL2_GLOBNAME(var,name) _ICANL2_GLOBNAME(var,name)
#else
# define ICANL2_GLOBNAME(var,name) _ICANL2_GLOBNAME(ICANL2,name)
#endif

#define  __ICANL2_GetEntry	  ICANL2_GLOBNAME(ICANL2_VARIANT,GetEntry)


#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
    extern void __ICANL2_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
 /* we have an MDIS4 men_types.h and mdis_api.h included */
 /* only 32bit compatibility needed!                     */
 #define INT32_OR_64  int32
 #define U_INT32_OR_64 u_int32
 typedef INT32_OR_64  MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
      }
#endif

#endif /* _ICANL2_DRV_H */

