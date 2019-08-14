/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2tb_init.c
 *      Project: ICANL2 host toolbox
 *
 *       Author: ub
 *        $Date: 2004/04/05 08:59:09 $
 *    $Revision: 1.10 $
 *
 *  Description: ICANL2 Toolbox: Init handle and hardware
 *
 *
 *     Required:
 *     Switches:
 *
 *---------------------------[ Public Functions ]----------------------------
 *  ICANL2TBOX_HandleSize, ICANL2TBOX_Init, ICANL2TBOX_Term
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2tb_init.c,v $
 * Revision 1.10  2004/04/05 08:59:09  ub
 * added variant for little endian and byteswap
 *
 * Revision 1.9  2003/02/06 13:36:33  ub
 * Comment changed.
 *
 * Revision 1.8  2002/02/04 15:16:10  ub
 * Cosmetics
 *
 * Revision 1.7  2001/11/29 12:00:08  kp
 * Added underscore to all function names (Variant specific renaming)
 *
 * Revision 1.6  2001/11/29 10:29:46  ub
 * In ICANL2TBOX_Init() distinction between M65 and P5 added.
 *
 * Revision 1.5  2001/11/21 10:54:37  ub
 * In ICANL2TBOX_Term() the two module processor on an M65 can now reset
 * separately.
 * ICANL2TBOX_Reset() drives the reset pin for 1000 module access cycles.
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2002 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#ifndef _LL_DRV_
 #include <string.h>
#endif

#include <MEN/men_typs.h>
#include <MEN/maccess.h>
#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_tbox.h>
#include <MEN/dbg.h>
#include <MEN/oss.h>

#include "icanl2tb_i.h"


/****************************** ICANL2TBOX_HandleSize ************************
 *
 *  Description: Get number of bytes required for ICANL2 handle
 *
 *				 This function returns the number of bytes required for
 *				 the toolbox handle	structure. The caller must allocate a
 *				 structure of sufficient size and must pass this pointer
 *				 to all toolbox functions.
 *---------------------------------------------------------------------------
 *  Input......: -
 *  Output.....: Return value:   size of handle
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_HandleSize(void)
{
	return sizeof(ICANL2TBOX_HANDLE);
}

/****************************** ICANL2TBOX_Init ******************************
 *
 *  Description: Initialize ICANL2 handle.
 *               Checks if M65 or P5 is accessed.
 *
 *				 Note: M-Module ID is not checked here. Do this externally
 *               if an M65 is used.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle (empty)
 *				 ma              module access descriptor in A08 mode
 *  Output.....: Return value:   error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_Init( ICANL2TBOX_HANDLE *h, MACCESS ma )
{
	int16 i;
	ICANL2_HANDLE *ch;

	h->ma = ma;
	h->magic = ICANL2_HANDLE_MAGIC;
	h->flags = 0;

	/* Is this an M65 module ? */
	if( MREAD_D16( ma, M65_CONFIG_REG ) & M65_DETC ) {

		h->can[0].winadrMsw = h->can[1].winadrMsw = M65_WINADDR_REG;
		h->can[0].winadrLsw = h->can[1].winadrLsw = M65_WINADDR_REG + 2;
		h->can[0].ptradrMsw = h->can[1].ptradrMsw = M65_PTRADDR_REG;
		h->can[0].ptradrLsw = h->can[1].ptradrLsw = M65_PTRADDR_REG + 2;

	/* If not assume a P5 module. Swap words, if necessary.*/
	} else {

#if ((defined (_LITTLE_ENDIAN_) && !(defined (MAC_BYTESWAP))) || \
    (defined (_BIG_ENDIAN_) && defined (MAC_BYTESWAP)))

		h->can[0].winadrMsw = h->can[1].winadrMsw = M65_WINADDR_REG + 2;
		h->can[0].winadrLsw = h->can[1].winadrLsw = M65_WINADDR_REG;
		h->can[0].ptradrMsw = h->can[1].ptradrMsw = M65_PTRADDR_REG + 2;
		h->can[0].ptradrLsw = h->can[1].ptradrLsw = M65_PTRADDR_REG;

#elif ((defined (_BIG_ENDIAN_) && !(defined (MAC_BYTESWAP))) || \
      (defined (_LITTLE_ENDIAN_) && defined (MAC_BYTESWAP)) )

		h->can[0].winadrMsw = h->can[1].winadrMsw = M65_WINADDR_REG;
		h->can[0].winadrLsw = h->can[1].winadrLsw = M65_WINADDR_REG + 2;
		h->can[0].ptradrMsw = h->can[1].ptradrMsw = M65_PTRADDR_REG;
		h->can[0].ptradrLsw = h->can[1].ptradrLsw = M65_PTRADDR_REG + 2;

#else
#error Invalid combination of _LITTLE_ENDIAN_, _BIG_ENDIAN_ and _BYTESWAP_ !
#endif
	}

	for( i=0, ch=h->can; i<2; ++i, ++ch ) {
		MACCESS_CLONE( ma, ch->ma, i ? 0x80 : 0x00 );
	}

	return( ICANL2TBOX_ERR_OK );
}

/****************************** ICANL2TBOX_Term ******************************
 *
 *  Description: De-initialize ICANL2
 *
 *			     This function stops CAN and disables interrupts.
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *  Output.....: Return value:	 error code
 *  Globals....: -
 ****************************************************************************/
int32
_ICANL2TBOX_Term(
	ICANL2TBOX_HANDLE *h,
    u_int8     canNum
)
{
	ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);

	/* Validate handle and return if not OK. */
	ICANL2_CHECK_HANDLE( h, canNum );

	/* Stop module processor and disable interrupt. */
	MWRITE_D16( ch->ma, M65_CONFIG_REG, M65_RST_CPU );

	return( ICANL2TBOX_ERR_OK );
}

/***************************** ICANL2TBOX_Reset ******************************
 *
 *  Description: Reset the processor on the M-Module
 *
 *---------------------------------------------------------------------------
 *  Input......: h               ICANL2 handle
 *               canNum          CAN channel (0=CAN-A, 1=CAN-B)
 *
 *  Output.....: Return value:   error code *
 *
 *  Globals....: -
 ****************************************************************************/
int32 _ICANL2TBOX_Reset(
    ICANL2TBOX_HANDLE  *h,
    u_int8     canNum
)
{
	u_int16 i;

	ICANL2_HANDLE *ch = GET_ICANL2_HANDLE(h, canNum);
	MACCESS ma = ch->ma;

	MSETMASK_D16( ma, M65_CONFIG_REG, M65_RST_CPU );

	/* The reset pin of the M68331 CPU must be driven for at least */
	/*  512 CPU cycles. */
	for( i=0; i<1000; ++i )
		MREAD_D16( ma, M65_PTRADDR_REG  );

	MCLRMASK_D16( ma, M65_CONFIG_REG, M65_RST_CPU );

	return( ICANL2TBOX_ERR_OK );
}


