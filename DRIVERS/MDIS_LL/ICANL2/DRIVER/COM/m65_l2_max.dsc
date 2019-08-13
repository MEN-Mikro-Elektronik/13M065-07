#************************** MDIS5 device descriptor *************************
#
#        Author: kp
#         $Date: 2010/03/08 15:41:01 $
#     $Revision: 1.2 $
#
#   Description: Metadescriptor for M65 with ICANL2 firmware
#
#****************************************************************************

#
# first half of an M65
#
M65_L2_1a  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   ICANL2  	# driver name

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   A201_1  	# device name of baseboard
    DEVICE_SLOT      = U_INT32  1           # used slot on baseboard (0..n)
	SUBDEVICE_OFFSET_0 = U_INT32 0x00		# first half of M65

	#------------------------------------------------------------------------
	#	debug levels (optional)
	#   this keys have only effect on debug drivers
	#------------------------------------------------------------------------
    DEBUG_LEVEL      = U_INT32  0xc0008003  # LL driver
    DEBUG_LEVEL_MK   = U_INT32  0xc0008000  # MDIS kernel
    DEBUG_LEVEL_OSS  = U_INT32  0xc0008000  # OSS calls
    DEBUG_LEVEL_DESC = U_INT32  0xc0008000  # DESC calls
    DEBUG_LEVEL_MBUF = U_INT32  0xc0008000  # MBUF calls

	#------------------------------------------------------------------------
	#	device parameters
	#------------------------------------------------------------------------
    IRQ_ENABLE       = U_INT32  1           # irq enabled after init
    ID_CHECK         = U_INT32  0           # check module ID prom
}

#
# second half of an M65
#
M65_L2_1b  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   ICANL2  	# driver name

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   A201_1  	# device name of baseboard
    DEVICE_SLOT      = U_INT32  1           # used slot on baseboard (0..n)
	SUBDEVICE_OFFSET_0 = U_INT32 0x80		# second half of M65

	#------------------------------------------------------------------------
	#	debug levels (optional)
	#   this keys have only effect on debug drivers
	#------------------------------------------------------------------------
    DEBUG_LEVEL      = U_INT32  0xc0008003  # LL driver
    DEBUG_LEVEL_MK   = U_INT32  0xc0008000  # MDIS kernel
    DEBUG_LEVEL_OSS  = U_INT32  0xc0008000  # OSS calls
    DEBUG_LEVEL_DESC = U_INT32  0xc0008000  # DESC calls
    DEBUG_LEVEL_MBUF = U_INT32  0xc0008000  # MBUF calls

	#------------------------------------------------------------------------
	#	device parameters
	#------------------------------------------------------------------------
    IRQ_ENABLE       = U_INT32  1           # irq enabled after init
    ID_CHECK         = U_INT32  0           # check module ID prom
}

