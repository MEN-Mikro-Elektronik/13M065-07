#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2002/02/04 15:13:38 $
#      $Revision: 1.4 $
#
#    Description: Makefile definitions for the M65/P5 ICANL2 driver (swapped)
#
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: driver_sw.mak,v $
#   Revision 1.4  2002/02/04 15:13:38  ub
#   icanl2_fwcode.h added
#
#   Revision 1.3  2001/12/20 10:32:12  Schoberl
#   added missing dependency icanl2tb_i.h
#
#   Revision 1.2  2001/12/12 16:24:18  kp
#   bug fix: use id_sw lib
#
#   Revision 1.1  2001/11/29 12:00:04  kp
#   Initial Revision
#
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=icanl2_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
		   $(SW_PREFIX)MAC_BYTESWAP \
		   $(SW_PREFIX)ICANL2_VARIANT=ICANL2_SW \
		   $(SW_PREFIX)ID_SW \
		   $(SW_PREFIX)PPC_GUARDED_ACCESS

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/id_sw$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)	\


MAK_INCL=$(MEN_INC_DIR)/icanl2_tbox.h \
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

