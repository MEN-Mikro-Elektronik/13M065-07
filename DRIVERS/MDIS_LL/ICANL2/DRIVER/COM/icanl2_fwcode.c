/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2_fwcode.c
 *      Project: M65 ICANL2 host toolbox
 *
 *      $Author: ub $ / kp
 *        $Date: 2004/04/05 08:58:58 $
 *    $Revision: 1.10 $
 *
 *  Description: M65 ICANL2 firmware code
 *
 *
 *     Required:
 *     Switches:
 *
 *---------------------------[ Public Functions ]----------------------------
 *  -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_fwcode.c,v $
 * Revision 1.10  2004/04/05 08:58:58  ub
 * cosmetics
 *
 * Revision 1.9  2002/02/04 15:14:49  ub
 * Cosmetics
 *
 * Revision 1.8  2001/12/20 10:32:23  Schoberl
 * firmware code now in icanl2_fwcode.h
 *
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/


#include <MEN/men_typs.h>
#include <MEN/maccess.h>
#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_tbox.h>   /* system dependent definitions   */
#ifdef __cplusplus
      extern "C" {
#endif

#define char u_int8
#include "icanl2_fwcode.h"

#ifdef __cplusplus
      }
#endif
