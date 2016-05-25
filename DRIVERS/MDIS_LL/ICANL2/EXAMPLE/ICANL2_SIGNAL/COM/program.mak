#***************************  M a k e f i l e  *******************************
#
#         Author: ub
#          $Date: 2004/04/05 08:59:29 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the ICANL2 signal example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2004/04/05 08:59:29  ub
#   removed unneccesary mdis_err.h from include files
#
#   Revision 1.1  2002/02/04 15:16:22  ub
#   Initial Revision
#
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2002 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

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