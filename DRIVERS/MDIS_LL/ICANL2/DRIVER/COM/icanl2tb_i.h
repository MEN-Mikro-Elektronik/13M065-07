/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: icanl2tb_i.h
 *
 *      Author: CRuff 
 *
 *  Description: Internal include file for M65 ICANL2 host toolbox library
 *
 *     Switches:
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

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
#define ICANL2_SHARED_RAM			0x8000		/* Start address of shared RAM*/
												/* (in module address space) */

#define MAX_HANDLE					(2048+20)	/* Biggest obj. handle + 1 */

#define ICANL2_FL_TRANSMIT_STROBE	0x01		/* Host has written data into */
												/*  fifo on module. */

/* Layout of the shared RAM (offsets) */
#if ((defined (_LITTLE_ENDIAN_) && !(defined (MAC_BYTESWAP))) || \
    (defined (_BIG_ENDIAN_) && defined (MAC_BYTESWAP)))

 #define ICANL2_FLAGS				0x01		/* Firmware flags        (rw)*/
 #define ICANL2_STATUS				0x00		/* Status (see #defines) (r) */
 #define ICANL2_LIVE_CNT			0x03		/* M65 alive.            (rw */
 #define ICANL2_HOST_SEMA			0x02		/* Host semaphore        (rw)*/
 #define ICANL2_SEMA				0x05		/* Firmware semaphore    (rw)*/
 #define ICANL2_COMMAND				0x04		/* Command to firmware.  (w) */
 #define ICANL2_RETVAL				0x07		/* Return value from FW. (r) */
 #define ICANL2_PARAM_LEN			0x06		/* Length of param. data (rw)*/
 #define ICANL2_OBJPTR_START		0x08		/* Start of ObjPtr array (r) */
 #define ICANL2_PARAMETER			0x0c		/* Parameter area.		 (rw)*/
 #define ICANL2_PARAM_MAX			(0x6e-ICANL2_PARAMETER)

#elif ((defined (_BIG_ENDIAN_) && !(defined (MAC_BYTESWAP))) || \
      (defined (_LITTLE_ENDIAN_) && defined (MAC_BYTESWAP)) )

 #define ICANL2_FLAGS				0x00		/* Firmware flags        (rw)*/
 #define ICANL2_STATUS				0x01		/* Status (see #defines) (r) */
 #define ICANL2_LIVE_CNT			0x02		/* M65 alive.            (rw)*/
 #define ICANL2_HOST_SEMA			0x03		/* Host semaphore        (rw)*/
 #define ICANL2_SEMA				0x04		/* Firmware semaphore    (rw)*/
 #define ICANL2_COMMAND				0x05		/* Command to firmware.  (w) */
 #define ICANL2_RETVAL				0x06		/* Return value from FW. (r) */
 #define ICANL2_PARAM_LEN			0x07		/* Length of param. data (rw)*/
 #define ICANL2_OBJPTR_START		0x08		/* Start of ObjPtr array (r) */
 #define ICANL2_PARAMETER			0x0c		/* Parameter area.		 (rw)*/
 #define ICANL2_PARAM_MAX			(0x6e-ICANL2_PARAMETER)

#else
#error Invalid combination of _LITTLE_ENDIAN_, _BIG_ENDIAN_ and _BYTESWAP_ !
#endif

/* #defines for the parameter area: */
/* Parameters for call and return. Six <int32> reserved for call and */
/*  two for return. After that generic data area intended for byte access. */
#define ICANL2_PARAM( n )			(ICANL2_PARAMETER+(n*4))
#define ICANL2_CALL_DATA( n )		(ICANL2_PARAMETER+(6*4)+n)
#define ICANL2_RET_DATA( n )		(ICANL2_PARAMETER+(2*4)+n)

/* M65 hardware registers: */
#define M65_WINADDR_REG			0x70			/* Window address */
#define M65_PTRACC_REG			0x76 			/* Pointer access */
#define M65_PTRADDR_REG			0x78			/* Pointer address */
#define M65_CONFIG_REG			0x7c			/* Config/status */

/* #defines for <CONFIG_REG>: */
#define M65_IRQ_HOST_EN			0x0004			/* Host interrupt enable. */
#define M65_IRQ_CPU				0x0008			/* Generate IRQ on M65/P5 CPU.*/
#define M65_IRQ_HOST			0x0010			/* Host IRQ pending. */
#define M65_DETC				0x0020			/* Always 1 on M65. */
#define M65_RST_CPU				0x0040			/* Reset M65 CPU. */

/* Magic word for <ICANL2TBOX_HANDLE.magic> */
#define ICANL2_HANDLE_MAGIC			0xab7c1403	/* Handle validation code */

/* #defines for <ICANL2_MO_DATAUSED>: */
#define ICANL2_DU_EMPTY				0			/* Buffer empty. */
#define ICANL2_DU_FULL				0xff		/* Buffer full. */
#define ICANL2_DU_COPYING			2			/* Copying in progress. */

/* Offsets for msg object control structures: */
#define ICANL2_MO_ID				0           /* CAN ID */
#define ICANL2_MO_TMODE				4           /* Transmit mode */
#define ICANL2_MO_TYPE				5           /* Type of object */
#define ICANL2_MO_FIFOPTR			6			/* Pointer to fifo struct. */
#define ICANL2_MO_HANDLE			10			/* Access handle. */
#define ICANL2_MO_DATA				12          /* Last received data. */
#define ICANL2_MO_DATALEN			20          /* Length of last rcvd. data. */
#define ICANL2_MO_DATAUSED			21          /* 1 -> ICANL2_MO_DATA used. */
#define ICANL2_MO_DATATIME			22			/* Time stamp of data. */

/* #defines for <ICANL2_FF_FLAGS>: */
#define ICANL2_FFL_FULL				0x0001		/* Fifo is full. */

/* Offsets for the fifo control structures: */
#define ICANL2_FF_ITEMPTR			0			/* List of items in fifo. */
#define ICANL2_FF_SIZE				4           /* Max. nbr. of entries. */
#define ICANL2_FF_RPOS				6           /* Read position. */
#define ICANL2_FF_WPOS				8           /* Write position. */
#define ICANL2_FF_SIGLVL			10          /* Nbr. entries to cause host */
                                                /*  interrupt. */
#define ICANL2_FF_FLAGS				12			/* Flags. (see #defines) */

#define ICANL2_STD_FIFO_SIZE		16			/* Size of std. msg fifo entry*/
#define ICANL2_GLB_FIFO_SIZE		24			/* Size of global/general/oob */
												/*  fifo entry. */

/* Offsets for the fifo entries: */
#define ICANL2_FE_FLAGS				0			/* Flags and handle. */
#define ICANL2_FE_DATA				2           /* Send/receive data. */
#define ICANL2_FE_TIME				10          /* Time of receival. */
#define ICANL2_FE_DATALEN			14          /* Length of data. */
#define ICANL2_FE_USED				15          /* 0xff: entry used.*/
#define ICANL2_FE_ID				16          /* Message ID (global/general */
												/*  msg fifo only) */

/* Offsets for the event fifo: */
#define ICANL2_EVF_ITEMPTR			0			/* List of items in fifo. */
#define ICANL2_EVF_SIZE				4           /* Max. nbr. of entries. */
#define ICANL2_EVF_RPOS				6           /* Write position. */
#define ICANL2_EVF_WPOS				8           /* Read position. */

/* Offsets for event fifo entries: */
#define ICANL2_EVE_TIME				0			/* Timestamp. */
#define ICANL2_EVE_HANDLE			4           /* Handle of object, that */
												/*  caused the event. */
#define ICANL2_EVE_EVENT			6           /* Event. */
#define ICANL2_EVE_USED				7           /* 0xff->entry used. */

/*--------------------------------------------------------------------------*/
/* Useful macros.*/
#define GET_ICANL2_HANDLE(h,canNum)	&h->can[canNum]

#define M65_GEN_IRQ_ON_MODULE(ma)	MSETMASK_D16(ma,M65_CONFIG_REG,M65_IRQ_CPU)

#define ICANL2_CHECK_HANDLE(h,canNum)	\
 if( !h || (h->magic!=ICANL2_HANDLE_MAGIC) || (canNum>=2)) \
   return ICANL2TBOX_ERR_BADHANDLE

// changed because of compiler warning (endless loop)
#define ICANL2_LOCK_SEM(ma)\
  MWRITE_D8(ma,ICANL2_HOST_SEMA,1); \
  while( MREAD_D8(ma,ICANL2_SEMA) ) { \
    MWRITE_D8(ma,ICANL2_HOST_SEMA,0); \
    MWRITE_D8(ma,ICANL2_HOST_SEMA,1); \
}

#define ICANL2_UNLOCK_SEM(ma)			MWRITE_D8(ma,ICANL2_HOST_SEMA,0)

/* The LSW of the M65 Pointer Address Register must be written before the MSW.*/
/* Otherwise errors may happen when accessing data. */
#define ICANL2_SET_POINTER( ch, adr )	\
					MWRITE_D16( (ch)->ma, (ch)->ptradrLsw, (adr) & 0xffff ); \
					MWRITE_D16( (ch)->ma, (ch)->ptradrMsw, (adr) >> 16 );

#define ICANL2_GET_POINTER( ch )	\
					(MREAD_D16( (ch)->ma, (ch)->ptradrLsw ) | \
					(MREAD_D16( (ch)->ma, (ch)->ptradrMsw ) << 16))

#define ICANL2_SET_WINADDR( ch, adr )	\
					MWRITE_D16( (ch)->ma, (ch)->winadrLsw, (adr) & 0xffff ); \
					MWRITE_D16( (ch)->ma, (ch)->winadrMsw, (adr) >> 16 );

#define ICANL2_GET_WINADDR( ch )	\
					(MREAD_D16( (ch)->ma, (ch)->winadrLsw ) | \
					(MREAD_D16( (ch)->ma, (ch)->winadrMsw ) << 16))
