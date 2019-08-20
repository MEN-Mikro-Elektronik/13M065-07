#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2002/02/04 15:13:35 $
#      $Revision: 1.4 $
#
#    Description: Makefile definitions for the M65/P5 ICANL2 driver
#
#
#-----------------------------------------------------------------------------
#   Copyright 2001-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

MAK_NAME=icanl2

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/id$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)	\


MAK_INCL= $(MEN_INC_DIR)/icanl2_tbox.h \
	 $(MEN_INC_DIR)/icanl2_drv.h	\
	 $(MEN_INC_DIR)/icanl2_codes.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/oss.h		\
         $(MEN_INC_DIR)/mdis_err.h	\
         $(MEN_INC_DIR)/maccess.h	\
         $(MEN_INC_DIR)/desc.h		\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/mdis_com.h	\
         $(MEN_INC_DIR)/modcom.h	\
         $(MEN_INC_DIR)/ll_defs.h	\
         $(MEN_INC_DIR)/ll_entry.h	\
         $(MEN_INC_DIR)/dbg.h		\
		 $(MEN_MOD_DIR)/icanl2tb_i.h \
		 $(MEN_MOD_DIR)/icanl2_fwcode.h

MAK_INP1=icanl2_drv$(INP_SUFFIX)
MAK_INP2=icanl2tb_init$(INP_SUFFIX)
MAK_INP3=icanl2tb_cmd$(INP_SUFFIX)
MAK_INP4=icanl2_fwcode$(INP_SUFFIX)


MAK_INP=$(MAK_INP1) \
        $(MAK_INP2) \
        $(MAK_INP3) \
        $(MAK_INP4) \

