#***************************  M a k e f i l e  *******************************
#  
#         Author: kp
#          $Date: 2001/11/29 12:00:22 $
#      $Revision: 1.1 $
#                      
#    Description: Makefile descriptor file for ICANL2_API lib
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

MAK_NAME=icanl2_api


MAK_INCL=$(MEN_INC_DIR)/men_typs.h    	\
		 $(MEN_INC_DIR)/mdis_err.h		\
         $(MEN_INC_DIR)/mdis_api.h		\
		 $(MEN_INC_DIR)/icanl2_api.h		\
		 $(MEN_INC_DIR)/icanl2_drv.h		\
		 $(MEN_INC_DIR)/icanl2_codes.h		\


MAK_INP1 = icanl2_api$(INP_SUFFIX)
MAK_INP2 = icanl2_strings$(INP_SUFFIX)

MAK_INP  = $(MAK_INP1) \
		   $(MAK_INP2)

