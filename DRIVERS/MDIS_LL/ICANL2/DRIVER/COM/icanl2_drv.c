/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2_drv.c
 *      Project: ICANL2 module driver (MDIS4)
 *
 *       Author: kp
 *
 *  Description: Low-level driver for M65/P5 modules using ICANL2 firmware
 *
 *               ICANL2 (Intelligent CAN Layer 2) is a firmware for
 *               M065 and P5 modules, which handles CAN Level 2 communication
 *               independently from host.
 *
 *               Most of the functionality is documented in icanl2_api.c
 *
 *
 *     Required: OSS, DESC, DBG, ID libraries
 *     Switches: _ONE_NAMESPACE_PER_DRIVER_
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


#include <MEN/men_typs.h>   /* system dependent definitions   */
#include <MEN/maccess.h>    /* hw access macros and types     */
#include <MEN/dbg.h>        /* debug functions                */
#include <MEN/oss.h>        /* oss functions                  */
#include <MEN/desc.h>       /* descriptor functions           */
#include <MEN/modcom.h>     /* ID PROM functions              */
#include <MEN/mdis_api.h>   /* MDIS global defs               */
#include <MEN/mdis_com.h>   /* MDIS common defs               */
#include <MEN/mdis_err.h>   /* MDIS error codes               */
#include <MEN/ll_defs.h>    /* low-level driver definitions   */

#include <MEN/icanl2_codes.h> /* ICANL2 firmware/toolbox defs */
#include <MEN/icanl2_tbox.h>  /* ICANL2 toolbox definitions  */
#include <MEN/ll_entry.h>     /* low-level driver jump table  */
#include <MEN/icanl2_drv.h>   /* ICANL2 driver header file */

#include "icanl2tb_i.h"

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* general */
#define CH_NUMBER			(2048+20)   /* number of device channels */
#define USE_IRQ				TRUE		/* interrupt required  */
#define ADDRSPACE_COUNT		1			/* nr of required address spaces */
#define ADDRSPACE_SIZE		128			/* size of address space */
#define MOD_ID_MAGIC		0x5346      /* ID PROM magic word */
#define MOD_ID_SIZE			128			/* ID PROM size [bytes] */
#define MOD_ID				65			/* ID PROM module ID */

#if defined(_BIG_ENDIAN_)
# define TO_BE16(x)			(x)
# define FROM_BE16(x)		(x)
# define TO_BE32(x)			(x)
# define FROM_BE32(x)		(x)
# define D16_TO_BE32( x )	(x)
# define D16_FROM_BE32( x )	(x)
#elif defined(_LITTLE_ENDIAN_)
# define TO_BE16(x)			OSS_SWAP16(x)
# define FROM_BE16(x)		OSS_SWAP16(x)
# define TO_BE32(x)			OSS_SWAP32(x)
# define FROM_BE32(x)		OSS_SWAP32(x)
# define D16_TO_BE32( x )	( (((x)>>16) & 0xffff) | (((x)<<16)&0xffff0000) )
# define D16_FROM_BE32( x )	D16_TO_BE32( x )
#else
# error "defined _BIG_ENDIAN_ or _LITTLE_ENDIAN_!"
#endif

/* debug settings */
#define DBG_MYLEVEL			h->dbgLevel
#define DBH					h->dbgHdl

#define	OSH					h->osHdl

/* LOCK_SEM - lock semaphore */
#define LOCK_SEM(sem) error = OSS_SemWait( OSH, sem, OSS_SEM_WAITFOREVER )

#define UNLOCK_SEM(sem) OSS_SemSignal( OSH, sem )

/* LOCK_DEVSEM - lock device semaphore */
#define LOCK_DEVSEM if( h->devSemHdl ) 	 LOCK_SEM(h->devSemHdl)
#define UNLOCK_DEVSEM if( h->devSemHdl ) UNLOCK_SEM(h->devSemHdl)

#define LOCK_CMDSEM if( h->cmdLockSem )   LOCK_SEM(h->cmdLockSem)
#define UNLOCK_CMDSEM if( h->cmdLockSem ) UNLOCK_SEM(h->cmdLockSem)

#define EVENT_FIFO_SIZE		256
/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/

/* low-level handle */
typedef struct {
	/* general */
    int32           memAlloc;		/* size allocated for the handle */
    OSS_HANDLE      *osHdl;         /* oss handle */
    OSS_IRQ_HANDLE  *irqHdl;        /* irq handle */
    DESC_HANDLE     *descHdl;       /* desc handle */
    MACCESS         ma;             /* hw access handle */
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/* id function table */
	/* debug */
    u_int32         dbgLevel;		/* debug level */
	DBG_HANDLE      *dbgHdl;        /* debug handle */
	/* misc */
    u_int32         irqCount;       /* interrupt counter */
    u_int32         idCheck;		/* id check enabled */
	OSS_SEM_HANDLE	*devSemHdl;		/* device semaphore */

	/*------- driver specific variables--------*/
	ICANL2TBOX_HANDLE tbHdl;		/* toolbox handle */
	OSS_SEM_HANDLE *cmdSem;			/* command finished semaphore */
	OSS_SEM_HANDLE *cmdLockSem;		/* command mutex semaphore
									   (created for OSes without device sem
									    only) */
	int16			rspLen;			/* length of data from firmware */
	u_int8			fwResult;		/* result code from firmware */
	u_int32 		reqData[20]; 	/* command request buffer */
	u_int32 		rspData[10]; 	/* command response buffer */

	/* event fifo */
	ICANL2_EVENT	evFifo[EVENT_FIFO_SIZE]; /* event fifo */
	ICANL2_EVENT	*evNxtIn;		/* fifo input ptr */
	ICANL2_EVENT	*evNxtOut;		/* fifo output ptr */
	u_int32			evCount;		/* num entries in fifo */
	OSS_SEM_HANDLE  *evSem;			/* counting event semaphore */
	OSS_SIG_HANDLE	*sigEvent;		/* signal for events */

} ICANL2DRV_HANDLE;


/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 ICANL2_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
					   MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
					   OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **hP);
static int32 ICANL2_Exit(LL_HANDLE **hP );
static int32 ICANL2_Read(LL_HANDLE *h, int32 ch, int32 *value);
static int32 ICANL2_Write(LL_HANDLE *h, int32 ch, int32 value);
static int32 ICANL2_SetStat(LL_HANDLE *h,int32 ch, int32 code, INT32_OR_64 value32_or_64);
static int32 ICANL2_GetStat(LL_HANDLE *h, int32 ch, int32 code,INT32_OR_64 *value32_or_64);
static int32 ICANL2_BlockRead(LL_HANDLE *h, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 ICANL2_BlockWrite(LL_HANDLE *h, int32 ch, void *buf, int32 size,
							 int32 *nbrWrBytesP);
static int32 ICANL2_Irq(LL_HANDLE *h );
static int32 ICANL2_Info(int32 infoType, ... );

static int32 SetRegister(ICANL2DRV_HANDLE *h, u_int8 regNr, u_int8 newVal );
static char* Ident( void );
static int32 Cleanup(ICANL2DRV_HANDLE *h, int32 retCode);
static int32 Command(
	ICANL2DRV_HANDLE *h,
	u_int8     opcode,
    u_int8     *reqData,
    int16      reqLen,
    u_int8     *rspData,
    int16      rspLen
);
static int32 MapTboxErr(int32 tboxerr);

/**************************** ICANL2_GetEntry *********************************
 *
 *  Description:  Initialize driver's jump table
 *
 *---------------------------------------------------------------------------
 *  Input......:  ---
 *  Output.....:  drvP  pointer to the initialized jump table structure
 *  Globals....:  ---
 ****************************************************************************/
#ifdef _ONE_NAMESPACE_PER_DRIVER_
    extern void LL_GetEntry( LL_ENTRY* drvP )
#else
    extern void __ICANL2_GetEntry( LL_ENTRY* drvP )
#endif
{
    drvP->init        = ICANL2_Init;
    drvP->exit        = ICANL2_Exit;
    drvP->read        = ICANL2_Read;
    drvP->write       = ICANL2_Write;
    drvP->blockRead   = ICANL2_BlockRead;
    drvP->blockWrite  = ICANL2_BlockWrite;
    drvP->setStat     = ICANL2_SetStat;
    drvP->getStat     = ICANL2_GetStat;
    drvP->irq         = ICANL2_Irq;
    drvP->info        = ICANL2_Info;
}


/******************************** ICANL2_Init ********************************
 *
 *  Description:  Allocate and return low-level handle, initialize hardware
 *
 *                The function initializes all channels with the
 *                definitions made in the descriptor. The interrupt
 *                is disabled.
 *
 *                The following descriptor keys are used:
 *
 *                Descriptor key        Default          Range
 *                --------------------  ---------------  -------------
 *                DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h
 *                DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h
 *                ID_CHECK              1                0..1
 *
 *				  ID_CHECK can be used only on the second half of an M65
 *				  (cannot be used on P5 or first M65 half)
 *---------------------------------------------------------------------------
 *  Input......:  descSpec   pointer to descriptor data
 *                osHdl      oss handle
 *                ma         hw access handle
 *                devSemHdl  device semaphore handle
 *                irqHdl     irq handle
 *  Output.....:  *hP     	 pointer to low-level driver handle
 *                return     success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Init(
    DESC_SPEC       *descP,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE       **_hP
)
{
    ICANL2DRV_HANDLE *h = NULL;
    u_int32 gotsize;
    int32 error, i;
    u_int32 value;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	*_hP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
    if ((h = (ICANL2DRV_HANDLE*)OSS_MemGet(
    				osHdl, sizeof(ICANL2DRV_HANDLE), &gotsize)) == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)h, 0x00);
	/* init */
    h->memAlloc   = gotsize;
    h->osHdl      = osHdl;
    h->irqHdl     = irqHdl;
    h->ma		  = *ma;
	h->devSemHdl  = devSemHdl;

	/*--- toolbox handle ---*/
	/* toolbox functions must be called always with <canNum> = 0, */
	if( (error = _ICANL2TBOX_Init( &(h->tbHdl), h->ma ) )) {
		return( Cleanup(h,error) );
	}

	/*--- command semaphore ---*/
	if( (error = OSS_SemCreate( OSH, OSS_SEM_BIN, 0, &h->cmdSem ))) {
		return( Cleanup(h,error) );
	}

	/*--- command mutex semaphore ---*/
	if( devSemHdl == NULL ){
		if( (error = OSS_SemCreate( OSH, OSS_SEM_BIN, 1, &h->cmdLockSem ))) {
			return( Cleanup(h,error) );
		}
	}

	/*--- event fifo init ---*/
	if( (error = OSS_SemCreate( OSH, OSS_SEM_COUNT, 0, &h->evSem ))) {
		return( Cleanup(h,error) );
	}
	h->evCount = 0;
	h->evNxtIn = h->evNxtOut = h->evFifo;

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* driver's ident function */
	h->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	h->idFuncTbl.idCall[1].identCall = DESC_Ident;
	h->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* fw ident ??? */
	/* terminator */
	h->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    if ((error = DESC_Init(descP, osHdl, &h->descHdl)))
		return( Cleanup(h,error) );

    /* DEBUG_LEVEL_DESC */
    if ((error = DESC_GetUInt32(h->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(h,error) );

	DESC_DbgLevelSet(h->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    if ((error = DESC_GetUInt32(h->descHdl, OSS_DBG_DEFAULT,
								&h->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(h,error) );

    DBGWRT_1((DBH, "LL - ICANL2_Init ma=%08p\n", (void*)h->ma));

    /* ID_CHECK */
    if ((error = DESC_GetUInt32(h->descHdl, TRUE,
								&h->idCheck, "ID_CHECK")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(h,error) );

    /*------------------------------+
    |  check module ID              |
    +------------------------------*/
	/* check only on second channel, assume ok for first channel */
	if (h->idCheck && (((U_INT32_OR_64)h->ma) & 0x80)) {
		int modIdMagic = (u_int16)m_read((U_INT32_OR_64)h->ma-0x80, 0);
		int modId      = (u_int16)m_read((U_INT32_OR_64)h->ma-0x80, 1);

		if (modIdMagic != MOD_ID_MAGIC) {
			DBGWRT_ERR((DBH," *** ICANL2_Init: illegal magic=0x%04x\n",
						modIdMagic));
			error = ERR_LL_ILL_ID;
			return( Cleanup(h,error) );
		}
		if (modId != MOD_ID) {
			DBGWRT_ERR((DBH," *** ICANL2_Init: illegal id=%d\n",modId));
			error = ERR_LL_ILL_ID;
			return( Cleanup(h,error) );
		}
	}

    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
	if( (error = _ICANL2TBOX_InitCom( &(h->tbHdl), 0 )))
		return( Cleanup(h,MapTboxErr(error)) );

	/* wait 1 sec max. for firmware to initialize */
	for( i=0; i<10; ++i ) {
		OSS_Delay( h->osHdl, 100 );
		if( _ICANL2TBOX_FwRunning( &(h->tbHdl), 0 ) )
			break;
	}

	/* timeout --> fw failure */
	if( i == 10 )
		return( Cleanup(h,ERR_LL_DEV_NOTRDY) );

	*_hP = (LL_HANDLE *)h;	/* set low-level driver handle */

	return(ERR_SUCCESS);
}

/****************************** ICANL2_Exit **********************************
 *
 *  Description:  De-initialize hardware and clean up memory
 *
 *                The function deinitializes all channels by setting the
 *                module processor in reset state. The interrupt is disabled.
 *
 *---------------------------------------------------------------------------
 *  Input......:  hP  		pointer to low-level driver handle
 *  Output.....:  return    success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Exit(
   LL_HANDLE    **_hP
)
{
    ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)*_hP;
	int32 error = 0;

    DBGWRT_1((DBH, "LL - ICANL2_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/
	_ICANL2TBOX_Term( &(h->tbHdl), 0 );

    /*------------------------------+
    |  clean up memory               |
    +------------------------------*/
	*_hP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(h,error);

	return(error);
}

/****************************** ICANL2_Read **********************************
 *
 *  Description:  not used
 *
 *---------------------------------------------------------------------------
 *  Input......:  h    low-level handle
 *                ch       current channel
 *  Output.....:  valueP   read value
 *                return   success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Read(
    LL_HANDLE *h,
    int32 ch,
    int32 *valueP
)
{
    return ERR_LL_ILL_FUNC;
}

/****************************** ICANL2_Write *********************************
 *
 *  Description:  not used
 *---------------------------------------------------------------------------
 *  Input......:  h    low-level handle
 *                ch       current channel
 *                value    value to write
 *  Output.....:  return   success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Write(
    LL_HANDLE *h,
    int32 ch,
    int32 value
)
{
    return ERR_LL_ILL_FUNC;
}

/****************************** ICANL2_SetStat *******************************
 *
 *  Description:  Set the driver status
 *
 * The following status codes are supported:
 *
 * Code                 	Description                 Values
 * -------------------  	--------------------------  ----------
 * ICANL2_DELETEOBJECT		see icanl2_api.c
 * ICANL2_ENABLECAN			see icanl2_api.c
 * ICANL2_INITCAN			see icanl2_api.c
 * ICANL2_SETTIMER          see icanl2_api.c
 * ICANL2_STARTTIMER		see icanl2_api.c
 * ICANL2_SYNCTIMER			see icanl2_api.c
 * ICANL2_SETSIG_EVENT		install signal for events	signal code
 * ICANL2_CLRSIG_EVENT		remove signal for events	-
 * ICANL2_BLK_SETTIMING		see icanl2_api.c
 * ICANL2_BLK_SETACCMASK	see icanl2_api.c
 * ICANL2_BLK_WRITEOBJECT	see icanl2_api.c
 * ICANL2_BLK_SENDCYCLIC	see icanl2_api.c
 *
 * M_LL_DEBUG_LEVEL     	driver debug level          see dbg.h
 * M_MK_IRQ_ENABLE      	interrupt enable            0..1
 * M_LL_IRQ_COUNT       	interrupt counter           0..max
 *
 * ICANL2_SETSIG_EVENT installs a signal that is sent when a new event
 *  arrives in the event fifo
 *
 * ICANL2_CLRSIG_EVENT removes the above signal
 *
 *---------------------------------------------------------------------------
 *  Input......:  h      	        low-level handle
 *                code              status code
 *                ch                current channel (ignored)
 *                value32_or_64     data or pointer 
 *                                  to block data structure (M_SG_BLOCK)  (*)
 *                                  (*) = for block status codes
 *  Output.....:  return     success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_SetStat(
    LL_HANDLE *_h,
    int32  code,
    int32  ch,
    INT32_OR_64 value32_or_64
)
{
	ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)_h;
	int32 error = ERR_SUCCESS;
    int32       value = (int32)value32_or_64;
    INT32_OR_64 valueP = value32_or_64;
    M_SG_BLOCK *blk = (M_SG_BLOCK*) valueP;

    DBGWRT_1((DBH, "LL - ICANL2_SetStat %08p: ch=%d code=0x%04x value=%08p\n",
			  (void*)h->ma, ch,code,value));

    switch(code) {

	case ICANL2_BLK_WRITEOBJECT:
	{
		ICANL2_WRITEOBJECT_PB *pb = (ICANL2_WRITEOBJECT_PB *)blk->data;
        u_int16 *p2;
        u_int32 *p1;
        int16 req_len;
		u_int8 dlen;

		if( blk->size != sizeof(ICANL2_WRITEOBJECT_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;

		/* issue ICANL2_CMD_SEND command */
		h->reqData[0] = D16_TO_BE32(pb->objHdl);
		h->reqData[1] = D16_TO_BE32((u_int32)pb->dataLen);
		h->reqData[2] = D16_TO_BE32((u_int32)pb->flags);

		dlen=(pb->dataLen+1) >> 1;
		for( p1=(u_int32 *)&h->reqData[6], p2=(u_int16 *)pb->data; dlen--; ) {
			*p1++ = TO_BE16( *p2 );
			++p2;
		}

        // check value range of data and cast if possible
        if ( 0x0 < ( ( p1 - (h->reqData) ) & 0xffff8000 ) ) {
            error = ERR_LL_ILL_PARAM;
            UNLOCK_CMDSEM;
            break;
        }
        req_len = (int16)( p1 - (h->reqData) );

		error = Command( h, ICANL2_CMD_SEND, (u_int8 *)h->reqData, req_len, NULL, 0 );
		UNLOCK_CMDSEM;

		break;
	}

	case ICANL2_BLK_SENDCYCLIC:
	{
		ICANL2_SENDCYCLIC_PB *pb = (ICANL2_SENDCYCLIC_PB *)blk->data;
		u_int32 *p1;
        u_int16 *p2;
        int16 req_len;
		u_int8 dlen;

		if( blk->size != sizeof(ICANL2_SENDCYCLIC_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;

		/* issue ICANL2_CMD_SEND_CYC command */
		h->reqData[0] = D16_TO_BE32(pb->objHdl);
		h->reqData[1] = D16_TO_BE32((u_int32)pb->dataLen);
		h->reqData[2] = D16_TO_BE32((u_int32)pb->cycleTime);

		dlen=(pb->dataLen+1) >> 1;
		for( p1=(u_int32 *)&h->reqData[6], p2=(u_int16 *)pb->data; dlen--; ) {
			*p1++ = TO_BE16( *p2 );
			++p2;
		}

        // check value range of data and cast if possible
        if ( 0x0 < ( ( p1 - (h->reqData) ) & 0xffff8000 ) ) {
            error = ERR_LL_ILL_PARAM;
            UNLOCK_CMDSEM;
            break;
        }
        req_len = (int16)( p1 - (h->reqData) );

		error = Command( h, ICANL2_CMD_SEND_CYC, (u_int8 *)h->reqData, 
                         req_len, NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_BLK_SETTIMING:
	{
        int32 regVal;
		ICANL2_SETTIMING_PB *pb= (ICANL2_SETTIMING_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_SETTIMING_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;

        regVal = (pb->sjw << 6) | pb->brp;
        if ( 0 < ( regVal & 0xffff0000 ) ) {
            error = ERR_LL_ILL_PARAM;
            UNLOCK_CMDSEM;
            break;
        }

        error = SetRegister( h, 0x3f, (u_int8)regVal );
		if( error ){
			UNLOCK_CMDSEM;
			break;
		}

        regVal = (pb->spl << 7) | (pb->tseg2 << 4) | pb->tseg1;
        if ( 0 < ( regVal & 0xffff0000 ) ) {
            error = ERR_LL_ILL_PARAM;
            UNLOCK_CMDSEM;
            break;
        }

        error = SetRegister( h, 0x4f, (u_int8)regVal );
		if( error ){
			UNLOCK_CMDSEM;
			break;
		}

		error = SetRegister( h, 0x2f, pb->busconf );

		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_BLK_SETACCMASK:
	{
		ICANL2_SETACCMASK_PB *pb= (ICANL2_SETACCMASK_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_SETACCMASK_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32(pb->accMask);
		h->reqData[1] = D16_TO_BE32((u_int32)pb->mode);

		error = Command( h, ICANL2_CMD_SETACCMASK, (u_int8 *)h->reqData,
						 2 * sizeof(u_int32), NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_DELETEOBJECT:
	{
		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32( (u_int32)value );
		error = Command( h, ICANL2_CMD_DELETEOBJ, (u_int8 *)h->reqData,
						 1 * sizeof(u_int32), NULL, 0);

		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_ENABLECAN:
	{
		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32( (u_int32)value );
		error = Command( h, ICANL2_CMD_ENABLECAN, (u_int8 *)h->reqData,
						 1 * sizeof(u_int32), NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_INITCAN:
	{
		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32( (u_int32)value );
		error = Command( h, ICANL2_CMD_INITCAN, (u_int8 *)h->reqData,
						 1 * sizeof(u_int32), NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_SETTIMER:
	{
		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32( (u_int32)value );
		error = Command( h, ICANL2_CMD_SETTIMER, (u_int8 *)h->reqData,
						 1 * sizeof(u_int32), NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_STARTTIMER:
	{
		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32( (u_int32)value );
		error = Command( h, ICANL2_CMD_STARTTIMER, (u_int8 *)h->reqData,
						 1 * sizeof(u_int32), NULL, 0);

		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_SYNCTIMER:
	{
		LOCK_CMDSEM; if( error ) break;

		error = Command( h, ICANL2_CMD_SYNCTIMER, NULL,
						 0, NULL, 0);
		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_SETSIG_EVENT:
		if( h->sigEvent ){
			error = ERR_OSS_SIG_SET; /* already installed */
			break;
		}
		error = OSS_SigCreate(h->osHdl, value, &h->sigEvent );
		break;

	case ICANL2_CLRSIG_EVENT:
		if( h->sigEvent == NULL ){
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(h->osHdl, &h->sigEvent );
		break;

	case M_LL_DEBUG_LEVEL:
		h->dbgLevel = value;
		break;

	case M_MK_IRQ_ENABLE:
		_ICANL2TBOX_IRQEnable( &h->tbHdl, 0, (u_int8)value );
		break;

	case M_LL_IRQ_COUNT:
		h->irqCount = value;
		break;

	default:
		error = ERR_LL_UNK_CODE;
		break;
    }

	return(error);
}

/****************************** ICANL2_GetStat ********************************
 *
 *  Description:  Get the driver status
 *
 * The following status codes are supported:
 *
 * Code                 	Description                 Values
 * -------------------  	--------------------------  ----------
 * ICANL2_FWIDENT			firmware revision
 * ICANL2_BLK_CREATEOBJECT	see icanl2_api.c
 * ICANL2_BLK_NEWESTMSG		see icanl2_api.c
 * ICANL2_BLK_GETEVENT		see icanl2_api.c
 * ICANL2_BLK_FIFOINFO      number of used entries
 * ICANL2_BLK_FWINFO		firmware status info
 * M_LL_DEBUG_LEVEL     	driver debug level           see dbg.h
 * M_LL_CH_NUMBER       	number of channels           2068
 * M_LL_ID_CHECK        	EEPROM is checked            0..1
 * M_LL_ID_SIZE         	EEPROM size [bytes]          128
 * M_LL_BLK_ID_DATA     	EEPROM raw data             -
 * M_MK_BLK_REV_ID      	ident function table ptr    -
 *
 *---------------------------------------------------------------------------
 *  Input......:  h      	 low-level handle
 *                code       status code
 *                ch         current channel (ignored)
 *                valueP     pointer to block data structure (M_SG_BLOCK)  (*)
 *                (*) = for block status codes
 *  Output.....:  value32_or_64P     data ptr or pointer
 *                                   to block data structure (M_SG_BLOCK)  (*)
 *                return     success (0) or error code
 *                (*) = for block status codes
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_GetStat(
    LL_HANDLE *_h,
    int32  code,
    int32  ch,
    INT32_OR_64  *value32_or_64P
)
{
	ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)_h;
    int32       *valueP     = (int32*)value32_or_64P; /* pointer to 32 bit value */
    INT32_OR_64 *value64P     = value32_or_64P;         /* stores 32/64bit pointer */
	M_SG_BLOCK  *blk        = (M_SG_BLOCK*)value32_or_64P;
	int32 error = ERR_SUCCESS;

    DBGWRT_1((DBH, "LL - ICANL2_GetStat %08p: ch=%d code=0x%04x\n",
			  (void*)h->ma, ch,code));

    switch(code)
    {

	case ICANL2_BLK_CREATEOBJECT:
	{
		ICANL2_CREATEOBJECT_PB *pb = (ICANL2_CREATEOBJECT_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_CREATEOBJECT_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;

		h->reqData[0] = D16_TO_BE32(pb->id);
		h->reqData[1] = D16_TO_BE32(pb->type);
		h->reqData[2] = D16_TO_BE32(pb->tMode);
		h->reqData[3] = D16_TO_BE32(pb->fifoSize);
		h->reqData[4] = D16_TO_BE32(pb->sigLvl);

		error = Command( h, ICANL2_CMD_CREATEOBJ, (u_int8 *)h->reqData,
						 5 * sizeof(u_int32), (u_int8 *)h->rspData,
						 sizeof(h->rspData));

		pb->objHdl = D16_FROM_BE32(h->rspData[0]);

		UNLOCK_CMDSEM;
		break;
	}

	case ICANL2_BLK_NEWESTMSG:
	{
		ICANL2_NEWESTMSG_PB *pb = (ICANL2_NEWESTMSG_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_NEWESTMSG_PB))
			return ERR_LL_ILL_PARAM;

		error = _ICANL2TBOX_NewestMsg( &h->tbHdl, 0, (u_int16)pb->objHdl,
									  &pb->msg.data_len, pb->msg.data,
									  &pb->msg.id, &pb->msg.time );
		pb->result = 0;

		if( error == ICANL2_E_NODATA ){
			pb->result = 1;		/* flag "no new data" */
			error = 0;
		}

		break;
	}

	case ICANL2_BLK_GETEVENT:
	{
		ICANL2_GETEVENT_PB *pb = (ICANL2_GETEVENT_PB *)blk->data;
		int32 errWait;

		if( blk->size != sizeof(ICANL2_GETEVENT_PB))
			return ERR_LL_ILL_PARAM;

		pb->event = 0;		/* flag "no event" */


		if( h->evCount == 0 && pb->timeout == OSS_SEM_NOWAIT )
			break;


		/* wait for event */
		UNLOCK_DEVSEM;
		errWait = OSS_SemWait( h->osHdl, h->evSem, pb->timeout );
		LOCK_DEVSEM;
		if( error ) break;

		error = errWait;

		if( error == ERR_OSS_TIMEOUT ){
			error = 0;			/* no event while waiting */
			pb->timeStamp = 0;
			break;
		}
		else if( error == 0 ){
			ICANL2_EVENT *e = h->evNxtOut;
            OSS_IRQ_STATE irqState;

			/* we got an event, copy to user */
			pb->objHdl 		= (u_int32)e->objHdl;
			pb->timeStamp	= e->timeStamp;
			pb->event  		= (u_int32)e->event;

			DBGWRT_1((DBH, "ICANL2_BLK_GETEVENT: pb->event=%x\n", pb->event ));

			irqState = OSS_IrqMaskR( h->osHdl, h->irqHdl );
			h->evCount--;
			OSS_IrqRestore( h->osHdl, h->irqHdl, irqState );

			if( ++h->evNxtOut == &h->evFifo[EVENT_FIFO_SIZE] )
				h->evNxtOut = h->evFifo;
		}

		break;
	}

	case ICANL2_BLK_FIFOINFO:
	{
		ICANL2_FIFOINFO_PB *pb = (ICANL2_FIFOINFO_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_FIFOINFO_PB))
			return ERR_LL_ILL_PARAM;

		error = _ICANL2TBOX_FifoInfo( &h->tbHdl, 0, (u_int16)pb->objHdl,
									  &(pb->numEntries) );
		break;
	}

	case M_LL_DEBUG_LEVEL:
		*valueP = h->dbgLevel;
		break;
	case M_LL_CH_NUMBER:
		*valueP = CH_NUMBER;
		break;
	case M_LL_IRQ_COUNT:
		*valueP = h->irqCount;
		break;
	case M_LL_ID_CHECK:
		*valueP = h->idCheck;
		break;
	case M_LL_ID_SIZE:
		*valueP = MOD_ID_SIZE;
		break;
	case M_LL_BLK_ID_DATA:
	{
		u_int32 n;
		u_int16 *dataP = (u_int16*)blk->data;

		if (blk->size < MOD_ID_SIZE)		/* check buf size */
			return(ERR_LL_USERBUF);

		for (n=0; n<MOD_ID_SIZE/2; n++)		/* read MOD_ID_SIZE/2 words */
			*dataP++ = (u_int16)m_read((U_INT32_OR_64)h->ma, (u_int8)n);

		break;
	}
	case M_MK_BLK_REV_ID:
		*value64P = (INT32_OR_64)&h->idFuncTbl;
		break;

	case ICANL2_FWIDENT:
		LOCK_CMDSEM; if( error ) break;
		if( error ) break;
		error = Command( h, ICANL2_CMD_IDENTIFY, NULL, 0,
							(u_int8*)(h->rspData), 2*sizeof( int32 ) );

		*valueP = D16_TO_BE32( h->rspData[1] );
       	UNLOCK_CMDSEM;
		DBGWRT_2( (DBH, "fw revision = %08x\n", *valueP ) );
		break;

	/*--------------------------+
	|  get firmware info        |
	+--------------------------*/
    case ICANL2_BLK_FWINFO:
   	{
		ICANL2_FWINFO_PB *pb = (ICANL2_FWINFO_PB *)blk->data;

		if( blk->size != sizeof(ICANL2_FWINFO_PB))
			return ERR_LL_ILL_PARAM;

		LOCK_CMDSEM; if( error ) break;
		if( error ) break;
		error = Command( h, ICANL2_CMD_GETINFO,
						NULL, 0,
						(u_int8*)h->rspData, 4 * sizeof( int32 ) );

		pb->numEvents = D16_TO_BE32( h->rspData[0] );	/* nbr events in fifo */
		pb->tick      = D16_TO_BE32( h->rspData[1] ); 	/* tick counter */
		pb->freeMem   = D16_TO_BE32( h->rspData[2] );	/* free mem */
		pb->numCyc    = D16_TO_BE32( h->rspData[3] );	/* nbr cyc objects */
       	UNLOCK_CMDSEM;
		break;
	}

	default:
		DBGWRT_1((DBH, "LL - ICANL2_GetStat:ERR_LL_UNK_CODE: code=0x%04x\n",
				  ch,code));
		error = ERR_LL_UNK_CODE;
    }

	return(error);
}

/******************************* ICANL2_BlockRead ****************************
 *
 *  Description:  Read a number of entries from an object's fifo
 *
 *  The current channel is used as the object handle.
 *
 *---------------------------------------------------------------------------
 *  Input......:  h        	   low-level handle
 *                ch           current channel
 *                buf          data buffer
 *                size         data buffer size
 *  Output.....:  nbrRdBytesP  number of read bytes
 *                return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_BlockRead(
     LL_HANDLE *_h,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrRdBytesP
)
{
	ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)_h;
	u_int32 numEntries = size/sizeof(ICANL2_DATA);
	int32 error;

    DBGWRT_1((DBH, "LL - ICANL2_BlockRead %08p: ch=%d, size=%d\n",
			  (void*)h->ma, ch,size));


	error = _ICANL2TBOX_ReadFifoEntry( &h->tbHdl, 0, (u_int16)ch,
									  (ICANL2_DATA *)buf, &numEntries );

	DBGWRT_2((DBH," ReadFifoEntry:error=%d numEntries=%d\n",
			  error, numEntries));
	if( error ){
		error = MapTboxErr( error );

		*nbrRdBytesP = 0;
	}
	else {
		*nbrRdBytesP = numEntries * sizeof(ICANL2_DATA);
	}

	return(error);
}

/****************************** ICANL2_BlockWrite ****************************
 *
 *  Description:  Write a number of entries to an object's fifo
 *
 *  The current channel is used as the object handle.
 *
 *---------------------------------------------------------------------------
 *  Input......:  h        	   low-level handle
 *                ch           current channel
 *                buf          data buffer
 *                size         data buffer size
 *  Output.....:  nbrWrBytesP  number of written bytes
 *                return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_BlockWrite(
     LL_HANDLE *_h,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
	ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)_h;
	u_int32 numEntries = size/sizeof(ICANL2_DATA);
	int32 error;

    DBGWRT_1((DBH, "LL - ICANL2_BlockWrite %08p: ch=%d, size=%d\n",
			  (void*)h->ma, ch,size));


	error = _ICANL2TBOX_WriteFifoEntry( &h->tbHdl, 0, (u_int16)ch,
									   (ICANL2_DATA *)buf, &numEntries );
	if( error ){
		error = MapTboxErr( error );

		*nbrWrBytesP = 0;
	}
	else {
		*nbrWrBytesP = numEntries * sizeof(ICANL2_DATA);
	}

	return(error);
}


/****************************** ICANL2_Irq ***********************************
 *
 *  Description:  Interrupt service routine
 *
 *                The interrupt is triggered by firmware. It indicates either
 *                the completion of a firmware command or an asynchronous
 *                event (e.g. fifo full).
 *
 *                If the driver can detect the interrupt's cause it returns
 *                LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *
 *---------------------------------------------------------------------------
 *  Input......:  h    low-level handle
 *  Output.....:  return   LL_IRQ_DEVICE	irq caused by device
 *                         LL_IRQ_DEV_NOT   irq not caused by device
 *                         LL_IRQ_UNKNOWN   unknown
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Irq(
   LL_HANDLE *_h
)
{
	ICANL2DRV_HANDLE *h = (ICANL2DRV_HANDLE *)_h;
	int32 retval = LL_IRQ_DEV_NOT;
	u_int8 reason, fwResult;
	u_int32 oldPointer;

    IDBGWRT_1((DBH, ">>> ICANL2_Irq: ma=%08p\n", (void*)h->ma));

	/* Read reason of IRQ from module. */
	reason = 0;
	if( _ICANL2TBOX_HandleIRQ( &h->tbHdl, 0,
							(ICANL2_STAT_CMD | ICANL2_STAT_EVENT),
							&reason, &fwResult ) == ICANL2TBOX_ERR_OK ) {

		if( reason ) {
			/*
			 * save the pointer address
			 * (will be restored after irq processing)
			 */
			oldPointer = ICANL2_GET_POINTER( &h->tbHdl.can[0] );

			if( reason & ICANL2_HI_EVENT_UPDATE ) {

				/* read event from firmware's event fifo into driver's fifo */

				IDBGWRT_2((DBH, " >event (evCount:%lu)\n", h->evCount ));

				while( h->evCount < EVENT_FIFO_SIZE ){
					u_int32 nEntries = 1;

					_ICANL2TBOX_ReadEvent( &h->tbHdl, 0, h->evNxtIn,
										  &nEntries );

					if( nEntries > 0 ){

						IDBGWRT_2((DBH, " >event 0x%x obj %d  time %d\n",
								   h->evNxtIn->event,
                                   h->evNxtIn->objHdl,
                                   h->evNxtIn->timeStamp ));

						if( ++h->evNxtIn == &h->evFifo[EVENT_FIFO_SIZE] )
							h->evNxtIn = h->evFifo;

						h->evCount++;
    					OSS_SemSignal( h->osHdl, h->evSem );
    					
    					/* send a signal if installed */
    					if( h->sigEvent )
    	   					OSS_SigSend( h->osHdl, h->sigEvent );

   					} else break;
				}
			}    /* end if( reason & ICANL2_HI_EVENT_UPDATE ) */

			if( reason & ICANL2_HI_CMD_FINISHED ) {
				IDBGWRT_2((DBH, " >cmd finished res=%d\n", fwResult ));

				/* Save firmware result. */
				h->fwResult = fwResult;
				/* Wake up driver. */
				OSS_SemSignal( OSH, h->cmdSem );
			}

			ICANL2_SET_POINTER( &h->tbHdl.can[0], oldPointer );

			retval = LL_IRQ_DEVICE;
		} /* if reason */
	}

	return( retval );
}

/****************************** ICANL2_Info ************************************
 *
 *  Description:  Get information about hardware and driver requirements
 *
 *                The following info codes are supported:
 *
 *                Code                      Description
 *                ------------------------  -----------------------------
 *                LL_INFO_HW_CHARACTER      hardware characteristics
 *                LL_INFO_ADDRSPACE_COUNT   nr of required address spaces
 *                LL_INFO_ADDRSPACE         address space information
 *                LL_INFO_IRQ               interrupt required
 *                LL_INFO_LOCKMODE          process lock mode required
 *
 *                The LL_INFO_HW_CHARACTER code returns all address and
 *                data modes (ORed) which are supported by the hardware
 *                (MDIS_MAxx, MDIS_MDxx).
 *
 *                The LL_INFO_ADDRSPACE_COUNT code returns the number
 *                of address spaces used by the driver.
 *
 *                The LL_INFO_ADDRSPACE code returns information about one
 *                specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *                data mode represents the widest hardware access used by
 *                the driver.
 *
 *                The LL_INFO_IRQ code returns whether the driver supports an
 *                interrupt routine (TRUE or FALSE).
 *
 *                The LL_INFO_LOCKMODE code returns which process locking
 *                mode the driver needs (LL_LOCK_xxx).
 *
 *---------------------------------------------------------------------------
 *  Input......:  infoType	   info code
 *                ...          argument(s)
 *  Output.....:  return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/
static int32 ICANL2_Info(
   int32  infoType,
   ...
)
{
    int32   error = ERR_SUCCESS;
    va_list argptr;

    va_start(argptr, infoType );

    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes ORed)   |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
			u_int32 *addrSizeP = va_arg(argptr, u_int32*);

			if (addrSpaceIndex >= ADDRSPACE_COUNT)
				error = ERR_LL_ILL_PARAM;
			else {
				*addrModeP = MDIS_MA08;
				*dataModeP = MDIS_MD16;
				*addrSizeP = ADDRSPACE_SIZE;
			}

			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_NONE;
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
}

/******************************* SetRegister ********************************
 *
 *  Description: Sets a single register of the i82627 on module.
 *
 *---------------------------------------------------------------------------
 *  Input......:  h      	 low-level handle
 *                regNr      number of i82627 register (0..255)
 *                newVal     new register value
 *
 *  Output.....:  return     success (0) or error code
 *  Globals....:  -
 ****************************************************************************/
static int32 SetRegister(						/* nodoc */
    ICANL2DRV_HANDLE *h,
	u_int8 regNr,
	u_int8 newVal
)
{
	h->reqData[0] = D16_TO_BE32((u_int32)regNr);
	h->reqData[1] = D16_TO_BE32((u_int32)newVal);

	return( Command( h, ICANL2_CMD_SETREG,
					(u_int8*)h->reqData, 2 * sizeof( u_int32 ),
					NULL, 0 ) );
}

/*******************************  Ident  ************************************
 *
 *  Description:  Return ident string
 *
 *---------------------------------------------------------------------------
 *  Input......:  -
 *  Output.....:  return  pointer to ident string
 *  Globals....:  -
 ****************************************************************************/
static char* Ident( void )	/* nodoc */
{
    return( (char*)IdentString );
}

/********************************* Cleanup **********************************
 *
 *  Description: Close all handles, free memory and return error code
 *		         NOTE: The low-level handle is invalid after this function is
 *                     called.
 *
 *---------------------------------------------------------------------------
 *  Input......: h		low-level handle
 *               retCode    return value
 *  Output.....: return	    retCode
 *  Globals....: -
 ****************************************************************************/
static int32 Cleanup(
   ICANL2DRV_HANDLE    *h,
   int32        retCode		/* nodoc */
)
{
    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if (h->descHdl)
		DESC_Exit(&h->descHdl);

	if( h->evSem )
		OSS_SemRemove( OSH, &h->evSem );

	if( h->cmdLockSem )
		OSS_SemRemove( OSH, &h->cmdLockSem );

	if( h->cmdSem )
		OSS_SemRemove( OSH, &h->cmdSem );

	if( h->sigEvent )
		OSS_SigRemove( OSH, &h->sigEvent );

	/* clean up debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(h->osHdl, (int8*)h, h->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
}

/********************************* Command ***********************************
 *
 *  Description: Send command to module firmware and wait for completion.
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: h			 	 low-level handle
 *				 opcode          command opcode
 *				 reqData         pointer to request data buffer
 *				 reqLen          length of <reqData> (in bytes)
 *				 rspData         pointer to response data block
 *				 *rspLenP        max. size of <rspData> (in bytes)
 *  Output.....: return			 MDIS error code
 *  Globals....: -
 ****************************************************************************/
static int32 Command(						/* nodoc */
	ICANL2DRV_HANDLE *h,
	u_int8     opcode,
    u_int8     *reqData,
    int16      reqLen,
    u_int8     *rspData,
    int16      rspLen
)
{
	int32 error = ERR_SUCCESS;

	DBGWRT_2((DBH," Command %08p: opcode=%x\n", (void*)h->ma, opcode ));

	/* Send command. */
	if( (error = _ICANL2TBOX_Command( &(h->tbHdl), 0, opcode, reqData,
									  reqLen, rspData, rspLen ) )) {
		DBGWRT_2((DBH," _ICANL2TBOX_Command() error=%x\n", error ));
		return( MapTboxErr( error ) );
	}

	/* Wait for command completion. */
	do {
		error = OSS_SemWait( OSH, h->cmdSem, 1000 ); /* wait 1s */
	} while( error==ERR_OSS_SIG_OCCURED );

	if( error != ERR_SUCCESS ) {
		DBGWRT_2((DBH," OSS_SemWait() error=%x\n", error ));
		return( error );
	}

	/* Convert toolbox return value. */
	return( MapTboxErr(h->fwResult) );
}

/******************************** MapTboxErr *********************************
 *
 *  Description: Convert toolbox error code to MDIS error code
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: tboxerr		toolbox error
 *  Output.....: return			MDIS error
 *  Globals....: -
 ****************************************************************************/
static int32 MapTboxErr(int32 tboxerr) /* nodoc */
{
	int32 error;

	switch(tboxerr){
	case ICANL2_E_OK:				error = ERR_SUCCESS; break;
	case ICANL2_E_CMD_UNKNOWN:   	error = ERR_LL_UNK_CODE; break;
	case ICANL2_E_FIFO_FULL:		error = ICANL2_ERR_FIFOFULL; break;
	case ICANL2_E_FIFO_EMPTY:		error = ICANL2_ERR_FIFOEMPTY; break;
	case ICANL2_E_NO_OBJ:			error = ICANL2_ERR_NOOBJ; break;

	case ICANL2_E_NO_MEM:			error = ICANL2_ERR_NOMEM; break;
	case ICANL2_E_OBJ_EXISTS:		error = ICANL2_ERR_OBJEXISTS; break;
	case ICANL2_E_NOT_FOUND:		error = ICANL2_ERR_NOTFOUND; break;
	case ICANL2_E_NO_TRANSMIT:		error = ERR_LL_ILL_DIR; break;

	case ICANL2_E_SIGLVL:			error = ICANL2_ERR_SIGLVL; break;
	case ICANL2_E_NODATA:			error = ICANL2_ERR_NODATA; break;
	case ICANL2_E_CYL_FULL:			error = ICANL2_ERR_CYL_FULL; break;
	case ICANL2_E_BUSY:				error = ERR_LL_DEV_BUSY; break;
	case ICANL2_E_COMM:				error = ERR_LL_DEV_BUSY; break;

	case ICANL2TBOX_ERR_NOTRDY:		error = ERR_LL_DEV_NOTRDY; break;
	case ICANL2TBOX_ERR_BUSY:		error = ERR_LL_DEV_BUSY; break;
	case ICANL2TBOX_ERR_BADHANDLE: 	error = ERR_LL_ILL_PARAM; break;
	case ICANL2TBOX_ERR_VERIFY:		error = ICANL2_ERR_VERIFY; break;

	default:						error = ICANL2_ERR_UNK_ERR; break;
	}

	return error;
}


