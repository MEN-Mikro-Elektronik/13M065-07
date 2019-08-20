#***************************  M a k e f i l e  *******************************
#
#         Author: ub
#          $Date: 2004/04/05 08:59:29 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the ICANL2 signal example program
#
#-----------------------------------------------------------------------------
#   Copyright 2002-2019, MEN Mikro Elektronik GmbH
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

MAK_NAME=icanl2_signal

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/icanl2_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
	     $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/icanl2_drv.h	\
		 $(MEN_INC_DIR)/icanl2_api.h	\
		 $(MEN_INC_DIR)/icanl2_codes.h	\
		 $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\

MAK_INP1=icanl2_signal$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
