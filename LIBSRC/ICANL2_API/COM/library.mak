#***************************  M a k e f i l e  *******************************
#  
#         Author: kp
#          $Date: 2001/11/29 12:00:22 $
#      $Revision: 1.1 $
#                      
#    Description: Makefile descriptor file for ICANL2_API lib
#                      
#---------------------------------[ History ]---------------------------------
#
#   $Log: library.mak,v $
#   Revision 1.1  2001/11/29 12:00:22  kp
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany 
#*****************************************************************************

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

