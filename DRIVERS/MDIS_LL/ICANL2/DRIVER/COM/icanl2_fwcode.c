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
