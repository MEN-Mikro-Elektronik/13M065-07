#************************** MDIS5 device descriptor *************************
#
#        Author: ub
#         $Date: 2010/03/08 15:41:20 $
#     $Revision: 1.2 $
#
#   Description: Metadescriptor for P5 with ICANL2 firmware
#
#****************************************************************************

P5_L2_SW_1  {
	#------------------------------------------------------------------------
	#	general parameters (don't modify)
	#------------------------------------------------------------------------
    DESC_TYPE        = U_INT32  1           # descriptor type (1=device)
    HW_TYPE          = STRING   ICANL2_SW  	# driver name

	PCI_VENDOR_ID	 = U_INT32 0x1172       # PCI vendor ID
	PCI_DEVICE_ID  	 = U_INT32 0x5005       # PCI device ID
	PCI_BASEREG_ASSIGN_0 = U_INT32 0        

	#------------------------------------------------------------------------
	#	reference to base board
	#------------------------------------------------------------------------
    BOARD_NAME       = STRING   F203_1  	# device name of baseboard
    DEVICE_SLOT      = U_INT32  1           # used slot on baseboard (0..n)
	SUBDEVICE_OFFSET_0 = U_INT32 0x00

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

