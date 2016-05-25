/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2tb_cmd.c
 *      Project: ICANL2 host toolbox
 *
 *       Author: ub
 *        $Date: 2009/06/30 11:45:51 $
 *    $Revision: 1.18 $
 *
 *  Description: ICANL2 Toolbox: Commands
 *
 *
 *     Required:
 *     Switches:
 *
 *---------------------------[ Public Functions ]----------------------------
 *
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2tb_cmd.c,v $
 * Revision 1.18  2009/06/30 11:45:51  CRuff
 * R: windows compiler warning after porting to MDIS5
 * M: added braces for type cast
 *
 * Revision 1.17  2009/06/29 15:49:55  CRuff
 * R: 1.Porting to MDIS5
 *    2.compiler warnings
 * M: 1.changed according to MDIS Porting Guide 0.5
 *    2.fixed compiler warings caused by type conversions etc.
 *
 * Revision 1.16  2004/06/04 13:01:07  kp
 * fixed compilation problem with GCC3 and _asm statements
 *
 * Revision 1.15  2004/04/05 08:59:06  ub
 * changed: ICANL2TBOX_LoadFirmware() declared static
 *
 * Revision 1.14  2002/02/07 14:14:02  ub
 * Fixed: Bug in calculation of number of free fifo entries in
 * ICANL2TBOX_WriteFifoEntry()
 * Fixed: <used> flag now set after all other elements of fifo entry in
 * ICANL2TBOX_WriteFifoEntry()
 *
 * Revision 1.13  2002/02/06 13:45:10  ub
 * Added: Modified MACCESS macros which synchronize the PPC's cache between moduleaccesses
 *
 * Revision 1.12  2002/02/04 15:16:08  ub
 * Added: Functionality for reading general message object using
 * ICANL2TBOX_NewestMsg().
 * Changed: Comment improvements.
 * Added: _ICANL2TBOX_FwRunning(), _ICANL2TBOX_WaitForFw()
 * Changed: ICANL2TBOX_InitCom() does no longer wait for initialization of
 * firmware.
 *
 * Revision 1.11  2001/12/20 10:32:33  Schoberl
 * fixed MSVC warnings
 * removed: external const char _ICANL2_fw
 *
 * Revision 1.10  2001/12/12 16:24:26  kp
 * fixed newest message to return consistent data
 *
 * Revision 1.9  2001/12/12 14:43:44  ub
 * ICANL2_SET_POINTER() statements necessary for Ultra-C now enclosed in #ifdefs.
 * Ported for little-endian CPUs.
 * _ICANL2_ReadFifo() now directly reads FIFO address from firmware RAM.
 * New: _ICANL2TBOX_FifoInfo()
 * _ICANL2TBOX_NewestMessage() now ensures data consistency.
 * _ICANL2TBOX_NewestMessage() returns time of receipt.
 *
 * Revision 1.8  2001/12/04 12:15:13  ub
 * Copy loops in ICANL2TBOX_WriteFifo() and ICANL2TBOX_ReadFifo() eliminated.
 *
 * Revision 1.7  2001/11/29 12:00:06  kp
 * Added underscore to all function names (Variant specific renaming)
 *
 * Revision 1.6  2001/11/29 10:29:44  ub
 * Pointer access changed to work on OS-9000/PPC.
 * Added: ICANL2TBOX_ReadEvent()
 * ICANL2TBOX_ReadFifo() and ICANL2TBOX_WriteFifo() extended to write/read
 * multiple FIFO entries.
 *
 * Revision 1.5  2001/11/21 10:54:35  ub
 * Some internal functions now flagged "nodoc".
 *
 * Revision 1.4  2001/11/06 16:40:40  ub
 * Added functions: ICANL2TBOX_WriteFifoEntry(), ICANL2TBOX_ReadFifoEntry(),
 * ICANL2TBOX_NewestMsg(), copy_to_module().
 *
 * Revision 1.3  2001/10/24 10:18:48  ub
 * ICANL2TOX_ReadFifo() newly implemented.
 * New function: ICANL2TBOX_ReadCommArea()
 * Replaced MREAD_D32()/MWRITE_D32() by MREAD_D16()/MWRITE_D16().
 *
 * Revision 1.2  2001/09/26 12:04:45  ub
 * Changed some function comments.
 *
 * Revision 1.1  2001/09/25 14:20:56  ub
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2002 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#ifndef _LL_DRV_
 #include <stdio.h>
#endif

#include <MEN/men_typs.h>
#include <MEN/maccess.h>
#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_tbox.h>
#include <MEN/dbg.h>
#include <MEN/oss.h>

#include "icanl2tb_i.h"


/* Special versions of some MACCESS macros. Intended for use with PowerPC-
   CPU boards that cause problems with cached access to module. This affects
   performance, so use only if really neccesary.
*/
#ifdef PPC_GUARDED_ACCESS

#undef MREAD_D8(ma,offs)
#undef MREAD_D16(ma,offs)
#undef MWRITE_D8(ma,offs,val)
#undef MWRITE_D16(ma,offs,val)

#define UCC_SYNC_EIEIO 	_asm( " sync;" \
			                  " eieio" )

u_int8 MREAD_D8( MACCESS ma, int offs )
{
	u_int8 ret;

	UCC_SYNC_EIEIO; 

	ret = (*(volatile u_int8* )((u_int8*)(ma)+(offs)));

	UCC_SYNC_EIEIO; 

	return( ret );
}

u_int16 MREAD_D16( MACCESS ma, int offs )
{
	u_int16 ret;

	UCC_SYNC_EIEIO; 

	ret = RSWAP16(*(volatile u_int16*)((u_int8*)(ma)+(offs)));

	UCC_SYNC_EIEIO; 

	return( ret );
}

void MWRITE_D8( MACCESS ma, int offs, u_int8 val)
{
	UCC_SYNC_EIEIO; 

	*(volatile u_int8* )((u_int8*)(ma)+(offs)) = val;

	UCC_SYNC_EIEIO; 
}

void MWRITE_D16( MACCESS ma, int offs, u_int16 val)
{
	UCC_SYNC_EIEIO; 

   *(volatile u_int16*)((u_int8*)(ma)+(offs)) = (u_int16)WSWAP16(val);

	UCC_SYNC_EIEIO; 
}

#endif /* PPC_GUARDED_ACCESS */


/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/

#if defined(_BIG_ENDIAN_)
# define TO_BE32(x)         (x)
# define FROM_BE32(x)       (x)
# define TO_BE16(x)         (x)
# define FROM_BE16(x)       (x)
# define D16_TO_BE32( x )   (x)
# define D16_FROM_BE32( x ) (x)
#elif defined(_LITTLE_ENDIAN_)
# define TO_BE32(x)         OSS_SWAP32(x)
# define FROM_BE32(x)       OSS_SWAP32(x)
# define TO_BE16(x)         OSS_SWAP16(x)
# define FROM_BE16(x)       OSS_SWAP16(x)
# define D16_TO_BE32( x )   ( (((x)>>16) & 0xffff) | (((x)<<16)&0xffff0000) )
# define D16_FROM_BE32( x ) D16_TO_BE32( x )
#else
# error "defined _BIG_ENDIAN_ or _LITTLE_ENDIAN_!"
#endif

#ifdef _UCC
# define POINTER_ACC_D32        pointer_access_d32( ma )
#else
# define POINTER_ACC_D32        ((MREAD_D16( ma, M65_PTRACC_REG ) << 16) | \
                                MREAD_D16( ma, M65_PTRACC_REG ))
#endif

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static void
copy_from_module(
    ICANL2_HANDLE *ch,
    u_int16 *host_adr,
    u_int32 module_adr,
    int32 num_bytes
);

static void
copy_to_module(
    ICANL2_HANDLE *ch,
    u_int32 module_adr,
    u_int16 *host_adr,
    int32 num_bytes
);

static u_int32
object_address(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    u_int16 handle
);

static u_int32
fifo_address(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    u_int16 handle
);

static int32
newest_msg_std(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    u_int16 handle,
    u_int8 *data_len_p,
    u_int8 *data_p,
    u_int32 *id_p,
    u_int32 *time_p
);

static int32
newest_msg_general(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    u_int16 handle,
    u_int8 *data_len_p,
    u_int8 *data_p,
    u_int32 *id_p,
    u_int32 *time_p
);

#ifdef _UCC             /* Ultra-C only. */
static u_int32
pointer_access_d32( MACCESS ma );
#endif

static int32
loadFirmware(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    const u_int8 *buffer,
    u_int32 size
);


/**************************** copy_from_module *******************************
 *
 *  Description: Copies data area from the module to host.
 *               Uses the pointer access mechanism of M-Modules.
 *               Host and module address and number of bytes to copy should
 *               be even. If not the function rounds up to the next even number.
 *---------------------------------------------------------------------------
 *  Input......: ma             access descriptor of module
 *               host_adr       data is copied to this address
 *               module_adr     address of source data on module. Must be even!
 *               num_bytes      number of bytes to copy. Must be even!
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static void
copy_from_module(   ICANL2_HANDLE *ch,                      /* nodoc */
                    u_int16 *host_adr,
                    u_int32 module_adr,
                    int32 num_bytes )
{
    /* Round up, if <num_bytes> is not even. */
    ++num_bytes;
    num_bytes >>= 1;

    ICANL2_SET_POINTER( ch, module_adr );

    while( num_bytes-- )  {
        *host_adr = MREAD_D16( ch->ma, M65_PTRACC_REG );
        ++host_adr;
    }
}

/***************************** copy_to_module *******************************
 *
 *  Description: Copies data area from host to module memory.
 *               Uses the pointer access mechanism of M-Modules.
 *               Host and module address and number of bytes to copy should
 *               be even. If not the function rounds up to the next even number.
 *---------------------------------------------------------------------------
 *  Input......: ma             access descriptor of module
 *               module_adr     address of source data on module. Must be even!
 *               host_adr       data is copied to this address
 *               num_bytes      number of bytes to copy. Must be even!
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static void
copy_to_module(     ICANL2_HANDLE *ch,                      /* nodoc */
                    u_int32 module_adr,
                    u_int16 *host_adr,
                    int32 num_bytes )
{
    /* Round up if <num_bytes> is not even. */
    ++num_bytes;
    num_bytes >>= 1;

    ICANL2_SET_POINTER( ch, module_adr );

    while( num_bytes-- )  {
        MWRITE_D16( ch->ma, M65_PTRACC_REG, *host_adr );
        ++host_adr;
    }
}

#ifdef _UCC
/**************************** pointer_access_d32 *****************************
 *
 *  Description: Reads 32-bit value from module via pointer access register.
 *               Only used for Microware Ultra-C compiler.
 *---------------------------------------------------------------------------
 *  Input......: ma             module access descriptor
 *  Output.....: return         see above
 *  Globals....: -
 ****************************************************************************/
static u_int32
pointer_access_d32( MACCESS ma )                                /* nodoc */
{
    volatile u_int16 low, high;

    high = MREAD_D16( ma, M65_PTRACC_REG );
    low =  MREAD_D16( ma, M65_PTRACC_REG );

    return( (high << 16) | low );
}
#endif /* _UCC */

/***************************** ICANL2TBOX_InitCom ***************************
 *
 *  Description: Initialize command communication with the ICANL2 Firmware.
 *               Loads firmware into module.
 *               After calling this routine you have to ensure that the
 *               firmware is running, e.g. using ICANL2TBOX_WaitForFw().
 *
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *  Output.....: Return value:   error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_InitCom( ICANL2TBOX_HANDLE *h, u_int8 canNum )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 size;

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Reset module processor. */
    MWRITE_D16(ma, M65_CONFIG_REG, 0x40);

    /* Load firmware. */
    size =  ((u_int8)_ICANL2_fw[0]<<24) |
            ((u_int8)_ICANL2_fw[1]<<16) |
            ((u_int8)_ICANL2_fw[2]<<8)  |
             (u_int8)_ICANL2_fw[3] ;

    if( loadFirmware( h, canNum, (u_int8*)&_ICANL2_fw[4], size ) !=
                                                    ICANL2TBOX_ERR_OK ) {
        return( ICANL2TBOX_ERR_VERIFY );
    }

    /* Set window pointer to start address of shared RAM area. */
    ICANL2_SET_WINADDR( ch, 0x8000 );

	/* Reset live counter, so if it is != 0 we can be sure that the FW */
	/* is running. */
    MWRITE_D8( ma, ICANL2_LIVE_CNT, 0 );

	/* Start processor. */
    MWRITE_D16(ma, M65_CONFIG_REG, 0x00);

    return( ICANL2TBOX_ERR_OK );
}

/**************************** ICANL2TBOX_FwRunning **************************
 *
 *  Description: Indicates, whether the firmware is running.
 *               Checks the firmware's live counter. This is always != 0
 *               after the firmware has initialized itself.
 *
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *  Output.....: Return value:   TRUE, if firmware running;
 *                               FALSE, if not.
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_FwRunning( ICANL2TBOX_HANDLE *h, u_int8 canNum )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;

	return( MREAD_D8( ma, ICANL2_LIVE_CNT ) != 0 );
}


/**************************** ICANL2TBOX_WaitForFw ***************************
 *
 *  Description: Wait for firmware to come up.
 *               Don't use this function in drivers. It uses a simple wait
 *               loop and so can lock up the host for the specified time.
 *
 *---------------------------------------------------------------------------
 *  Input......: h              ICANL2 handle
 *               canNum         CAN channel (0=CAN-A, 1=CAN-B)
 *               ms				Number of milliseconds to wait.
 *  Output.....: Return value	error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_WaitForFw( ICANL2TBOX_HANDLE *h, u_int8 canNum , int32 ms )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    int32 i;

    /* This assumes an access time of 50ns. */
    for( i=0; i<2000*ms; ++i ) {
        if( MREAD_D8( ma, ICANL2_LIVE_CNT ) != 0 ) {
            return( ICANL2TBOX_ERR_OK );
        }
    }

    return( ICANL2TBOX_ERR_NOTRDY );
}

/***************************** ICANL2TBOX_Command ***************************
 *
 *  Description: Send a command to the ICANL2 Firmware
 *
 *  This function also sets up variables to receive response data from the
 *  M65/P5 module. The routine does not wait for completion of the command.
 *  It returns immediately after the first command packet was placed into the
 *  M65/P5 RAM. Command completion is flagged by the interrupt routine.
 *
 *  The data buffers reqData and rspData must be kept intact until
 *  ICANL2TBOX_HandleIrq() returns the ICANL2_HI_CMD_FINISHED status.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               opcode          command opcode
 *               reqData         pointer to request data buffer
 *               reqLen          length of <reqData> (in bytes)
 *               rspData         pointer to reponse data block
 *               rspLen          max. size of <rspData> (in bytes)
 *
 *  Output.....: Return value:   error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_Command(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum,
    u_int8     opcode,
    u_int8     *reqData,
    int16      reqLen,
    u_int8     *rspData,
    int16      rspLen
)
{
    int16 i, off;
    u_int16 *data;
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* If there is already a command pending, return error. */
    if( MREAD_D8(ch->ma, ICANL2_COMMAND ) ) {
        return ICANL2TBOX_ERR_BUSY;
    }

    ch->reqData = reqData;                      /* setup data/length */
    ch->reqLen  = reqLen;
    ch->rspData = rspData;
    ch->rspLen  = rspLen;
    ch->opcode  = opcode;

    /* Write parameters of the command. */
    MWRITE_D8(ch->ma, ICANL2_PARAM_LEN, (u_int8)ch->reqLen );

    data = (u_int16*)reqData;
    for( i=0, off=ICANL2_PARAMETER; i<reqLen; i+=2, off+=2, ++data ) {
        MWRITE_D16(ch->ma, off, *data );
    }

    /* Set opcode. The module firmware starts processing now. */
    MWRITE_D8(ch->ma, ICANL2_COMMAND, ch->opcode );

    return( ICANL2TBOX_ERR_OK );
}

/************************** ICANL2TBOX_HandleIRQ ****************************
 *
 *  Description: Check for a pending interrupt on the specified CAN channel
 *
 *               The routine only clears the module IRQ flag, if neither
 *               an event nor a command completion is pending.
 *               Using <statusMask> you can determine if event update or
 *               command completion are read.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               statusMask      determines if event update or command
 *                               completion (or both) are read
 *                               ICANL2_STAT_CMD
 *                                   read command completion
 *                               ICANL2_STAT_EVENT
 *                                   read event update
 *                               (ICANL2_STAT_CMD | ICANL2_STAT_EVENT)
 *                                   read both
 *
 *  Output.....: Return value:   error code
 *
 *               *resultP        several flags:
 *                               ICANL2_HI_IRQ_RECEIVED
 *                                   just for info. An interrupt from
 *                                   the module has been received
 *                               ICANL2_HI_CMD_FINISHED
 *                                    command finished (and response
 *                                    data received)
 *                               ICANL2_HI_EVENT_UPDATE
 *                                    events updated
 *
 *              *fwResultP       result value from firmware (may be NULL)
 *
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_HandleIRQ(
    ICANL2TBOX_HANDLE  *h,
    u_int8    canNum,
    u_int8    statusMask,
    u_int8    *resultP,
    u_int8    *fwResultP
)
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    int16 i, off;
    u_int8 status;
    u_int16 *data, rspLen;

    ICANL2_CHECK_HANDLE(h, canNum);

    /* Is there an irq pending from module? */
    if( !((status=(u_int8)MREAD_D16( ma, M65_CONFIG_REG ))
            & M65_IRQ_HOST) ) {
        return( ICANL2TBOX_ERR_OK );
    }

    /* Clear irq by SETTING the IRQ_HOST bit. */
    MSETMASK_D16( ma, M65_CONFIG_REG, M65_IRQ_HOST );

    *resultP = ICANL2_HI_IRQ_RECEIVED;

    /* Read status register. */
    status = MREAD_D8( ma, ICANL2_STATUS );

    /* What happened? */
    if( status & statusMask & ICANL2_STAT_EVENT ){
        /* Event from firmware: */
        *resultP |= ICANL2_HI_EVENT_UPDATE;

        ICANL2_LOCK_SEM(ma);
        /* Clear <STAT_EVENT> to enable firmware to mark new event. */
        MCLRMASK_D8( ch->ma, ICANL2_STATUS, ICANL2_STAT_EVENT );
        ICANL2_UNLOCK_SEM(ma);
    }

    if( status & statusMask & ICANL2_STAT_CMD ) {

        /* Firmware command completed: */
        ICANL2_LOCK_SEM(ma);
        MCLRMASK_D8( ch->ma, ICANL2_STATUS, ICANL2_STAT_CMD );
        ICANL2_UNLOCK_SEM(ma);

        *resultP |= ICANL2_HI_CMD_FINISHED;

        if( fwResultP ) {
            *fwResultP = MREAD_D8( ma,ICANL2_RETVAL );
        }

        /* Limit length of response to given maximum.  */
        rspLen = MREAD_D8( ch->ma, ICANL2_PARAM_LEN );
        if( rspLen > ch->rspLen )  {
            rspLen = ch->rspLen;
        }

        /* Copy response data. */
        data = (u_int16*)ch->rspData;

        for( i=0, off=ICANL2_PARAMETER; i<rspLen; i+=2, off+=2, ++data ) {
            *data = MREAD_D16(ch->ma, off);
        }
    }

    return( ICANL2TBOX_ERR_OK );
}

/************************** ICANL2TBOX_IRQEnable ****************************
 *
 *  Description: Enable/disable global M-Module interrupt
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               enable          TRUE enables interrupt,
 *                               FALSE disables
 *
 *  Output.....: Return value:   error code *
 *
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_IRQEnable(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum,
    u_int8     enable
)
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;

    if( enable ) {
        MSETMASK_D16( ma, M65_CONFIG_REG, M65_IRQ_HOST_EN );
    } else {
        MCLRMASK_D16( ma, M65_CONFIG_REG, M65_IRQ_HOST_EN );
    }

    return( ICANL2TBOX_ERR_OK );
}

/****************************** loadFirmware *******************************
 *
 *  Description: Load the firmware into one channel of the M65/P5
 *               Firmware is loaded to address 0.
 *               ATTENTION! Module processor must be in reset state when
 *               this routine is called.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               buffer          contains firmware to be loaded
 *               size            size of firmware in bytes
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static int32                                        /* nodoc */
loadFirmware(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    const u_int8 *buffer,
    u_int32 size )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int16 val, i;

    /* Firmware starts at address 0. Set pointer address. */
    ICANL2_SET_POINTER( ch, 0 );
    for( i=0; i<size; i+=2 ) {
        val = (u_int16) (buffer[i]<<8 | buffer[i+1]);
        MWRITE_D16( ma, M65_PTRACC_REG, val );
    }

    /* Verify firmware. */
    ICANL2_SET_POINTER( ch, 0 );
    for( i=0; i<size; i+=2 ) {
        val = (u_int16) (buffer[i]<<8 | buffer[i+1]);
        if( val != MREAD_D16( ma, M65_PTRACC_REG ) )  {
            return( ICANL2TBOX_ERR_VERIFY );
        }
    }

    /* Patch firmware start address into MENMON. */
    ICANL2_SET_POINTER( ch, 0x400 );
    MWRITE_D16( ma, M65_PTRACC_REG, 0x0000 );
    MWRITE_D16( ma, M65_PTRACC_REG, 0x8200 );

    return( ICANL2TBOX_ERR_OK );
}

/************************* ICANL2TBOX_ReadFifo ******************************
 *
 *  Description: Read the complete FIFO of a message object.
 *               FIFO layout for standard FIFO entry:
 *                u_int16  flags;       lowest 14 bits: msg object handle
 *                u_int8   data[8];     data received/to be transmitted
 *                u_int32  time;        time of receipt
 *                int8     data_len;    actual length of data
 *                u_int8   own;         owner (0x00=host)
 *
 *               FIFO layout for global/general FIFO entry:
 *                u_int16  flags;       lowest 14 bit: msg object handle
 *                u_int8   data[8];     data received/to be transmitted
 *                u_int32  time;        time of receipt
 *                int8     data_len;    actual length of data
 *                u_int8   own;         owner (0x00=host)
 *                u_int32  id;          message ID
 *                u_int32  reserved;
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          handle of firmware message object
 *               entrySize       size in bytes of one FIFO entry
 *                                - standard FIFO: 16
 *                                - global/general FIFO: 24
 *  Output.....: objCnt          number of returned CAN frames
 *               data            pointer to FIFO data. Must be big enough to
 *                                contain the entire FIFO.
 *                                Size depends on type of FIFO:
 *                                - standard FIFO: 16 * <max_entries>
 *                                - global/general FIFO: 24 * <max_entries>
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_ReadFifo( ICANL2TBOX_HANDLE *h,
                        u_int8 canNum,
                        u_int16 handle,
                        u_int16 *objCnt,
                        u_int8 *data )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 fifo_adr, data_adr, used_adr;
    int16 entries_read, rpos, fifo_size, entry_size;

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Select size of FIFO entries. */
    if( (handle == ICANL2_GLOBAL_HANDLE) ||
        (handle == ICANL2_GENERAL_HANDLE) ||
        (handle == ICANL2_OOB_HANDLE) ) {

        entry_size = 24;

    } else {
        entry_size = 16;
    }

    /* Get start address of FIFO struct. */
    fifo_adr = fifo_address( h, canNum, handle );

    if( fifo_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_ITEMPTR );

    /* Get some basic data about the FIFO: */
    /* Start address of entry array. */
    data_adr = POINTER_ACC_D32;

    /* Size of FIFO (max. number of entries). */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_SIZE );
#endif
    fifo_size = MREAD_D16( ma, M65_PTRACC_REG );

    /* Current read position. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
#endif
    rpos = MREAD_D16( ma, M65_PTRACC_REG );

    entries_read = 0;

    /* Read all used entries from <rpos>. */
    for(;;) {

        /* Is this entry being used? */
        used_adr = data_adr + rpos * entry_size + ICANL2_FE_DATALEN;
        ICANL2_SET_POINTER( ch, used_adr );
        if( (MREAD_D16( ma, M65_PTRACC_REG ) & 0xff) != 0xff ) {
            break;
        }

        /* Read entry. */
        copy_from_module( ch, (u_int16*)data,
                            (data_adr + rpos * entry_size),
                            entry_size );

        /* Mark this entry as cleared. */
        /* Set fields <data_len> and <used> of FIFO struct to zero.*/
        ICANL2_SET_POINTER( ch, used_adr );
        MWRITE_D16( ma, M65_PTRACC_REG, 0 );

        /* Next entry. */
        ++entries_read;
        rpos = (rpos+1) % fifo_size;
        data += entry_size;

        /* Update <rpos>. */
        ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
        MWRITE_D16( ma, M65_PTRACC_REG, rpos );
    }

    /* Pass number of entries to calling routine. */
    *objCnt = entries_read;

    return( ICANL2_E_OK );
}

/************************* ICANL2TBOX_ReadCommArea ***************************
 *
 *  Description: Read the second 256 bytes of the host comunication area
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *  Output.....: dst             pointer to destination array
 *                               Must have a size of at least 256 bytes.
 *  Globals....: -
 ****************************************************************************/
void
_ICANL2TBOX_ReadCommArea(
    ICANL2TBOX_HANDLE *h,
    u_int8 canNum,
    u_int8 *dst )
{
    u_int16 val;
    int16 i;

    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE( h, canNum );
    MACCESS ma = ch->ma;

    ICANL2_SET_POINTER( ch, ICANL2_SHARED_RAM + 256 );

    for( i=0; i<256; i+=2 ) {
        val = MREAD_D16( ma, M65_PTRACC_REG );
        dst[i] = (u_int8)(val>>8);
        dst[i+1] = ((u_int8)val) & 0xff;
    }
}

/****************************** object_address *******************************
 *
 *  Description: Return the start address of a message object
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          handle of object to get address
 *
 *  Output.....: return          address of object in firmware space
 *  Globals....: -
 ****************************************************************************/
static u_int32
object_address( ICANL2TBOX_HANDLE *h,               /* nodoc */
                u_int8 canNum,
                u_int16 handle )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 objptr_start, objptr, obj_adr;

    /* Read base address of message objects. */
    objptr_start = ( MREAD_D16( ch->ma, ICANL2_OBJPTR_START ) << 16) |
                     MREAD_D16( ch->ma, ICANL2_OBJPTR_START + 2 );

    /* Access object with <handle>. */
    objptr = objptr_start + handle*4;

    ICANL2_SET_POINTER( ch, objptr );

    /* Get object address. */
    obj_adr = POINTER_ACC_D32;

    return( obj_adr );
}

/****************************** fifo_address ********************************
 *
 *  Description: Return the start address of a FIFO control structure
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          handle of object to get FIFO address
 *
 *  Output.....: return          address of FIFO in firmware space
 *  Globals....: -
 ****************************************************************************/
static u_int32
fifo_address( ICANL2TBOX_HANDLE *h,                 /* nodoc */
                u_int8 canNum,
                u_int16 handle )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 obj_adr, fifo_adr;

    /* Handle in correct range? */
    if( handle >= ICANL2_MAX_HANDLE ) {
        return( 0 );
    }

    /* Get object address. */
    obj_adr = object_address( h, canNum, handle );

    /* Return 0, if address invalid. */
    if( obj_adr == 0 ) {
        return( 0 );
    }

    /* What kind of object? */
    /* Standard message object. */
    if( handle >= ICANL2_FIRST_STD_HANDLE ) {

        /* Get FIFO address from the object struct. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_FIFOPTR );
        fifo_adr = POINTER_ACC_D32;

    /* Global/general/oob msg object. */
    } else if( handle <= ICANL2_OOB_HANDLE ) {

        /* These object types are handled using a FIFO struct. */
        fifo_adr = obj_adr;

    /* All other object types are not accessible by this function. */
    } else {

        fifo_adr = 0;
    }

    return( fifo_adr );
}

/************************** ICANL2TBOX_WriteFifoEntry ***********************
 *
 *  Description: Send CAN frames
 *
 *               Writes frames directly into the module's transmit FIFO.
 *
 *               Performed actions:
 *               - Read base address of message objects from command area.
 *               - Find message object in module memory.
 *               - Read start address of FIFO.
 *               - Append frame to FIFO.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          Handle of firmware message object.
 *               *data            Data to send.
 *               *numEntries     Size of data[] (Number of entries).
 *
 *  Output.....: return          error code
 *               *numEntries     Number of sent frames
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_WriteFifoEntry( ICANL2TBOX_HANDLE *h,
                            u_int8 canNum,
                            u_int16 handle,
                            ICANL2_DATA *data,
                            u_int32 *numEntries )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 fifo_adr, data_adr, item_adr;
    int32 n;
    u_int16 rpos, wpos, fifo_size, tx_flags, i;
    u_int8 entry_size, tmpData[24];


    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Select size of FIFO entries. */
    if( (handle == ICANL2_GLOBAL_HANDLE) ||
        (handle == ICANL2_GENERAL_HANDLE) ||
        (handle == ICANL2_OOB_HANDLE) ) {

        entry_size = 24;

    } else {
        entry_size = 16;
    }

    /* Get start address of FIFO struct. */
    fifo_adr = fifo_address( h, canNum, handle );

    if( fifo_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    /* Get start address of FIFO entries. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_ITEMPTR );
    data_adr = POINTER_ACC_D32;

    /* Read size of FIFO. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_SIZE );
#endif
    fifo_size = MREAD_D16( ma, M65_PTRACC_REG );

    /* Read <rpos>. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
#endif
    rpos = MREAD_D16( ma, M65_PTRACC_REG );

    /* Read <wpos>. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_WPOS );
#endif
    wpos = MREAD_D16( ma, M65_PTRACC_REG );

	/* Compute number of free entries in fifo. */
    n = wpos - rpos - 1;

    if( n == 0 )
    	n = fifo_size - 2;
    else if( n < 0 )
        n += fifo_size;

    if( *numEntries < (u_int32)n )
        n = *numEntries;


    for( *numEntries=0; *numEntries < (u_int32)n; ++(*numEntries), ++data ) {

        /* Calculate address of FIFO entry. */
        item_adr = data_adr + (wpos * entry_size);

        /* Prepare data set. */
        tx_flags = (u_int16)TO_BE16( data->tx_flags );
        tmpData[ICANL2_FE_FLAGS] = (u_int8) (tx_flags >> 8);
        tmpData[ICANL2_FE_FLAGS+1] = (u_int8)tx_flags;

        for( i=0; i<8; i+=2 ) {
            *(u_int16*)&(tmpData[ICANL2_FE_DATA+i]) =
                                        (u_int16)TO_BE16( *(u_int16*)&(data->data[i]) );
        }

        /* Set <datalen> and <used> to zero for now. */
        tmpData[ICANL2_FE_DATALEN] = 0x00;
        tmpData[ICANL2_FE_USED] = 0x00;

        /* If available, set CAN ID. */
        if( entry_size == 24 ) {

            *(u_int16*)&(tmpData[ICANL2_FE_ID]) = (u_int16)(data->id >> 16);
            *(u_int16*)&(tmpData[ICANL2_FE_ID+2]) = (u_int16)data->id;
        }

        /* Write FIFO entry into module FIFO. */
        copy_to_module( ch, item_adr, (u_int16*)&tmpData, entry_size );

        /* Set <datalen> and <used> last. */
        tmpData[ICANL2_FE_DATALEN] = data->data_len;
        tmpData[ICANL2_FE_USED] = 0xff;

		ICANL2_SET_POINTER( ch, item_adr + ICANL2_FE_DATALEN );
		MWRITE_D16( ma, M65_PTRACC_REG,
			TO_BE16( *(u_int16*)&(tmpData[ICANL2_FE_DATALEN])) );

        /* Increment write position. */
        if( ++wpos >= fifo_size ) {
            wpos = 0;
        }
    }

    /* Update write pointer. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_WPOS );
    MWRITE_D16( ma, M65_PTRACC_REG, wpos );

    /* Inform module about new FIFO entry. */
    MWRITE_D8( ma, ICANL2_FLAGS, ICANL2_FL_TRANSMIT_STROBE );

    return( ICANL2TBOX_ERR_OK );
}

/************************** ICANL2TBOX_ReadFifoEntry ************************
 *
 *  Description: Read a number of received CAN frames
 *
 *               Data is read directly from the module's receive FIFO.
 *
 *               Performed actions:
 *               - Read base address of message objects from command area.
 *               - Find message object in module memory.
 *               - Read start address of FIFO.
 *               - Read frame from FIFO.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          Handle of firmware message object.
 *               *numEntries     Size of data[] (Number of entries).
 *
 *  Output.....: data            Received data.
 *                               <data->id> only used with global
 *                                and general FIFO
 *               *numEntries     Number of returned frames
 *
 *               return          error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_ReadFifoEntry( ICANL2TBOX_HANDLE *h,
                            u_int8 canNum,
                            u_int16 handle,
                            ICANL2_DATA *data,
                            u_int32 *numEntries )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 fifo_adr, data_adr, item_adr, n;
    u_int16 rpos, fifo_size, i;
    u_int8 entry_size, tmpData[24];

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Select size of FIFO entries. */
    if( (handle == ICANL2_GLOBAL_HANDLE) ||
        (handle == ICANL2_GENERAL_HANDLE) ||
        (handle == ICANL2_OOB_HANDLE) ) {

        entry_size = 24;

    } else {
        entry_size = 16;
    }

    /* Get start address of FIFO struct. */
    fifo_adr = fifo_address( h, canNum, handle );

    if( fifo_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    /* Get start address of FIFO entries. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_ITEMPTR );
    data_adr = POINTER_ACC_D32;

    /* Read size of FIFO. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_SIZE );
#endif
    fifo_size = MREAD_D16( ma, M65_PTRACC_REG );

    /* Clear "FIFO full" flag. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_FLAGS );
    MWRITE_D16( ma, M65_PTRACC_REG, 0 );

    /* Read <rpos>. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
    rpos = MREAD_D16( ma, M65_PTRACC_REG );

    n = *numEntries;
    for( *numEntries=0; *numEntries < n; ++(*numEntries), ++data ) {

        /* Is the next entry being used? */
        item_adr = data_adr + (rpos * entry_size);
        ICANL2_SET_POINTER( ch, item_adr + ICANL2_FE_DATALEN );

        /* If no, write back read position and return. */
        if( (MREAD_D16( ma, M65_PTRACC_REG ) & 0x00ff) != 0xff ) {
            break;
        }

        /* Read data from module. */
        copy_from_module( ch, (u_int16*)tmpData, item_adr, entry_size );

        /* Copy data. */
        data->data_len = (u_int8)((*(u_int16*)&(tmpData[ICANL2_FE_DATALEN]) ) >> 8);

        for( i=0; i<8; i+=2 ) {
            *(u_int16*)&(data->data[i]) =
                        (u_int16)FROM_BE16( *(u_int16*)&(tmpData[ICANL2_FE_DATA+i]) );
        }

        data->time = (*(u_int16*)&(tmpData[ICANL2_FE_TIME]) << 16) |
                      ( *(u_int16*)&(tmpData[ICANL2_FE_TIME+2]));

        /* If available, get CAN ID. */
        if( entry_size == 24 ) {
            data->id = (*(u_int16*)&(tmpData[ICANL2_FE_ID]) << 16) |
                        (*(u_int16*)&(tmpData[ICANL2_FE_ID+2]));
        } else {
            data->id = 0;
        }

        /* Clear <datalen> and <used>. */
        ICANL2_SET_POINTER( ch, item_adr + ICANL2_FE_DATALEN );
        MWRITE_D16( ma, M65_PTRACC_REG, 0 );

        /* Increment read position. */
        if( ++rpos >= fifo_size ) {
            rpos = 0;
        }
    }   /* end for( *numEntries=0;...  */

    /* Write back read position. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
    MWRITE_D16( ma, M65_PTRACC_REG, rpos );

    return( ICANL2TBOX_ERR_OK );
}

/*************************** ICANL2TBOX_NewestMsg ****************************
 *
 *  Description: Read the newest received CAN message from a msg object
 *
 *               Works only with standard message objects (handle >=
 *               ICANL2_FIRST_STD_HANDLE).
 *
 *               Performed actions:
 *               - Read base address of message objects from command area.
 *               - Find message object in module memory.
 *               - Read frame from message object.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          handle of firmware message object
 *
 *  Output.....: data_len_p      length of CAN data to be received
 *               data_p          CAN data
 *               id_p            CAN ID (may be NULL)
 *               time_p          time stamp of rcvd data (may be NULL)
 *               return          error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_NewestMsg( ICANL2TBOX_HANDLE *h,
                        u_int8 canNum,
                        u_int16 handle,
                        u_int8 *data_len_p,
                        u_int8 *data_p,
                        u_int32 *id_p,
                        u_int32 *time_p )
{
    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Works with standard message objects and the general msg obj. only. */
    if( handle == ICANL2_GENERAL_HANDLE ) {
        return( newest_msg_general( h, canNum, handle, data_len_p, data_p,
                id_p, time_p ) );

    } else if( IN_RANGE( handle, ICANL2_FIRST_STD_HANDLE, ICANL2_MAX_HANDLE-1)){
        return( newest_msg_std( h, canNum, handle, data_len_p, data_p,
                id_p, time_p ) );
    }
    return( ICANL2_E_NOT_FOUND );
}

/*--------------------------------------------------------------------------*/
static int32
newest_msg_std( ICANL2TBOX_HANDLE *h,
                u_int8 canNum,
                u_int16 handle,
                u_int8 *data_len_p,
                u_int8 *data_p,
                u_int32 *id_p,
                u_int32 *time_p )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int16 data_len, used, i;
    u_int32 obj_adr;

    /* Get start address of msg object struct. */
    obj_adr = object_address( h, canNum, handle );

    if( obj_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    /* Repeat until data is considered consistent. */
    do {
        /* Read length of data and "buffer state" indicator. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_DATALEN );
        data_len = MREAD_D16( ma, M65_PTRACC_REG );

        used = data_len & 0xff;

        /* Anything received? */
        if( used != ICANL2_DU_FULL ) {
            return( ICANL2_E_NODATA );
        }

        *data_len_p = (u_int8) (data_len >> 8);

        /* Mark data as being copied. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_DATALEN );
        MWRITE_D16( ma, M65_PTRACC_REG, ICANL2_DU_COPYING );

        /* Read CAN ID. */
        if( id_p ) {
            ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_ID );
            *id_p = POINTER_ACC_D32;
        }

        /* Read CAN time. */
        if( time_p ) {
            ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_DATATIME );
            *time_p = POINTER_ACC_D32;
        }

        /* Read data. */
        copy_from_module( ch, (u_int16*)data_p, obj_adr + ICANL2_MO_DATA,
                        *data_len_p );

        for( i=0; i<8; i+=2 ) {
            *(u_int16*)&(data_p[i]) = (u_int16)FROM_BE16(*(u_int16*)&(data_p[i]) );
        }

        /* Check if the firmware has changed the data in the meantime. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_DATALEN );
    } while( (MREAD_D16( ma, M65_PTRACC_REG ) & 0xff) != ICANL2_DU_COPYING );

    /* Mark data as read. */
    ICANL2_SET_POINTER( ch, obj_adr + ICANL2_MO_DATALEN );
    MWRITE_D16( ma, M65_PTRACC_REG, ICANL2_DU_EMPTY );

    return( ICANL2_E_OK );
}

/*--------------------------------------------------------------------------*/
static int32
newest_msg_general( ICANL2TBOX_HANDLE *h,
                    u_int8 canNum,
                    u_int16 handle,
                    u_int8 *data_len_p,
                    u_int8 *data_p,
                    u_int32 *id_p,
                    u_int32 *time_p )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int16 data_len, used, i;
    u_int32 obj_adr;

    /* Get start address of the last received message.*/
    obj_adr = object_address( h, canNum, ICANL2_LASTMSG_HANDLE );

    if( obj_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    /* Repeat until data is considered consistent. */
    do {
        /* Read length of data and "buffer state" indicator. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_DATALEN );
        data_len = MREAD_D16( ma, M65_PTRACC_REG );

        used = data_len & 0xff;

        /* Return if nothing received or firmware is updating data now. */
        if( used != ICANL2_DU_FULL ) {
            return( ICANL2_E_NODATA );
        }

        *data_len_p = (u_int8) (data_len >> 8);

        /* Mark data as being copied. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_DATALEN );
        MWRITE_D16( ma, M65_PTRACC_REG, ICANL2_DU_COPYING );

        /* Read CAN ID. */
        if( id_p ) {
            ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_ID );
            *id_p = POINTER_ACC_D32;
        }

        /* Read CAN ID. */
        if( time_p ) {
            ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_TIME );
            *time_p = POINTER_ACC_D32;
        }

        /* Read data. */
        copy_from_module( ch, (u_int16*)data_p, obj_adr + ICANL2_FE_DATA,
                        *data_len_p );

        for( i=0; i<8; i+=2 ) {
            *(u_int16*)&(data_p[i]) = (u_int16)FROM_BE16(*(u_int16*)&(data_p[i]) );
        }

        /* Check if the firmware has changed the data in the meantime. */
        ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_DATALEN );
    } while( (MREAD_D16( ma, M65_PTRACC_REG ) & 0xff) != ICANL2_DU_COPYING );

    /* Mark data as read. */
    ICANL2_SET_POINTER( ch, obj_adr + ICANL2_FE_DATALEN );
    MWRITE_D16( ma, M65_PTRACC_REG, ICANL2_DU_EMPTY );

    return( ICANL2_E_OK );
}


/**************************** ICANL2TBOX_ReadEvent ***************************
 *
 *  Description: Read events from the module's event FIFO
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               *numEntries     size of data[] (Number of entries)
 *
 *  Output.....: *ev             event
 *               *numEntries     number of returned frames
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_ReadEvent(  ICANL2TBOX_HANDLE *h,
                        u_int8 canNum,
                        ICANL2_EVENT *ev,
                        u_int32 *numEntries )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 fifo_adr, data_adr, item_adr, n;
    u_int16 rpos, fifo_size;
    u_int8 entry_size = 8;

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Get start address of FIFO struct. */
    fifo_adr = fifo_address( h, canNum, ICANL2_EVENT_HANDLE );

    if( fifo_adr == 0 ) {
        return( ICANL2_E_FIFO_EMPTY );
    }

    /* Get start address of FIFO entries. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_EVF_ITEMPTR );
    data_adr = POINTER_ACC_D32;

#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_EVF_SIZE );
#endif
    /* Read size of FIFO. */
    fifo_size = MREAD_D16( ma, M65_PTRACC_REG );

    /* Read <rpos>. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_EVF_RPOS );
#endif
    rpos = MREAD_D16( ma, M65_PTRACC_REG );

    n = *numEntries;
    for( *numEntries=0; *numEntries < n; ++(*numEntries), ++ev ) {

        /* Is the next entry being used? */
        item_adr = data_adr + (rpos * entry_size);

        ICANL2_SET_POINTER( ch, item_adr + ICANL2_EVE_EVENT );

        /* If no, write back read position and return. */
        if( ((MREAD_D16( ma, M65_PTRACC_REG )) & 0x00ff) != 0xff ) {
            break;
        }

        /* Read data from module. */
        copy_from_module( ch, (u_int16*)ev, item_adr, entry_size );

        /* Swap 16-bit and 32-bit values, if needed. */
        ev->timeStamp = D16_FROM_BE32( ev->timeStamp );

        *(u_int16*)&(ev->event) = (u_int16)TO_BE16(*(u_int16*)&(ev->event));

        /* Clear <event> and <used>. */
        ICANL2_SET_POINTER( ch, item_adr + ICANL2_EVE_EVENT );
        MWRITE_D16( ma, M65_PTRACC_REG, 0 );

        /* Increment read position. */
        if( ++rpos >= fifo_size ) {
            rpos = 0;
        }
    }   /* end for( *numEntries=0;...  */

    /* Write back read position. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_EVF_RPOS );
    MWRITE_D16( ma, M65_PTRACC_REG, rpos );

    return( ICANL2_E_OK );
}

/*************************** ICANL2TBOX_FifoInfo ****************************
 *
 *  Description: Return the number of entries of an object's FIFO
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *               handle          handle of firmware message object
 *
 *  Output.....: return          error code
 *               *numEntries     number of entries in FIFO
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_FifoInfo( ICANL2TBOX_HANDLE *h,
                        u_int8 canNum,
                        u_int16 handle,
                        int32 *numEntries )
{
    ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
    MACCESS ma = ch->ma;
    u_int32 fifo_adr;
    u_int16 rpos, wpos, fifo_size;

    /* Validate handle and return if not OK. */
    ICANL2_CHECK_HANDLE( h, canNum );

    /* Get start address of FIFO struct. */
    fifo_adr = fifo_address( h, canNum, handle );

    if( fifo_adr == 0 ) {
        return( ICANL2_E_NOT_FOUND );
    }

    /* Read size of FIFO. */
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_SIZE );
    fifo_size = MREAD_D16( ma, M65_PTRACC_REG );

    /* Read <rpos>. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_RPOS );
#endif
    rpos = MREAD_D16( ma, M65_PTRACC_REG );

    /* Read <wpos>. */
#ifdef _UCC
    ICANL2_SET_POINTER( ch, fifo_adr + ICANL2_FF_WPOS );
#endif
    wpos = MREAD_D16( ma, M65_PTRACC_REG );

    *numEntries = wpos - rpos;
    if( *numEntries < 0 ) {
        *numEntries += fifo_size;
    }

    return( ICANL2_E_OK );
}


