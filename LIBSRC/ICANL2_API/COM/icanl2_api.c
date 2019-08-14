/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: icanl2_api.c
 *      Project: M65/P5 intelligent layer 2 driver
 *
 *       Author: kp
 *        $Date: 2009/06/29 16:00:43 $
 *    $Revision: 1.6 $
 *
 *  Description: Application interface for ICANL2 driver
 *
 *     Required: ICANL2 MDIS4 LL driver
 *     Switches: -
 *
 *---------------------------[ Public Functions ]----------------------------
 *
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: icanl2_api.c,v $
 * Revision 1.6  2009/06/29 16:00:43  CRuff
 * R: 1.Porting to MDIS5
 * M: 1.changed according to MDIS Porting Guide 0.5
 *
 * Revision 1.5  2004/04/05 08:59:37  ub
 * added function comment
 *
 * Revision 1.4  2002/02/07 14:16:59  ub
 * Comment changed
 *
 * Revision 1.3  2002/02/04 15:16:27  ub
 * Added: ICANL2API_FwIdent(), ICANL2API_FwInfo(), ICANL2API_Signal()
 * Corrected: Comment of ICANL2API_SetTimer()
 * Deleted: ICANL2API_InitCan()
 *
 * Revision 1.2  2001/12/12 14:46:42  ub
 * ICANL2API_FifoInfo() added
 *
 * Revision 1.1  2001/11/29 12:00:23  kp
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2001 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include <MEN/men_typs.h>
#include <MEN/mdis_err.h>
#include <MEN/mdis_api.h>
#include <MEN/icanl2_codes.h>
#include <MEN/icanl2_drv.h>
#include <MEN/icanl2_api.h>

/********************************* ICANL2API_SetTiming ************************
 *
 *  Description: Set timing and bus parameters
 *
 *	This routine directly sets the corresponding registers of the CAN chip
 *  It must be called before CAN communication is enabled via
 *  ICANL2API_EnableCan()!
 *
 *  The following table lists suggested paramters to be used for different
 *  baudrates. However the parameters may differ depending for special
 *  requirements:
 *
 *      baudrate		brp		sjw		tseg1	tseg2	 spl
 *      ------------    -----   -----   ------  ------   ---
 *		{ 1000000,        0,     1,       4,       1,    0 },
 *		{  500000,        1,     1,       4,       1,    0 },
 *		{  250000,        3,     1,       4,       1,    0 },
 *		{  125000,        7,     1,       4,       1,    0 },
 *		{  100000,        9,     1,       4,       1,    0 },
 *		{   50000,       19,     1,       4,       1,    0 },
 *		{   20000,       49,     1,       4,       1,    0 },
 *		{  	10000,       39,     1,       13,      4,    0 },
 *
 *  Please refer to the I82527 data sheet for more information
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 brp		baudrate prescaler
 *				 sjw		synchronisation jump with
 *				 tseg1		timing segment 1
 *				 tseg2		timing segment 2
 *				 spl		sampling point
 *               busconf	bus configuration register
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_SetTiming(
	MDIS_PATH path,
	u_int8 brp,
	u_int8 sjw,
	u_int8 tseg1,
	u_int8 tseg2,
	u_int8 spl,
	u_int8 busconf )
{
	ICANL2_SETTIMING_PB pb;
	M_SG_BLOCK blk;

	pb.brp 		= brp;
	pb.sjw		= sjw;
	pb.tseg1	= tseg1;
	pb.tseg2	= tseg2;
	pb.spl		= spl;
	pb.busconf	= busconf;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	return M_setstat( path, ICANL2_BLK_SETTIMING, (INT32_OR_64)&blk );
}

/********************************* ICANL2API_SetAccMask **********************
 *
 *  Description: Set global acceptance mask and extended/basic CAN mode
 *
 *	This routine sets the global acceptance mask. It influences the behaviour
 *  of RTR objects only.
 *  A "0" value at a specific bit position means "accept a 0 or 1 for that bit".
 *  A "1" means that the bit value of the incoming frame must match
 *  identically to the corresponding bit in the RTR object's CAN ID.
 *
 *  If more than one RTR object's ID matches the pattern, then the one
 *  with the lower handle will answer.
 *
 *  The global decision wether to run in extended (29bit IDs) or basic
 *  CAN (11 bit IDs) is also done using this function.
 *
 *	This function can be called during operation.
 *  If this function is not called, extended mode is set with an acceptance
 *  mask of 0x1FFFFFFF
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 accMask	acceptance mask
 *				 mode		0=basic 1=extended
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_SetAccMask(
	MDIS_PATH path,
	u_int32 accMask,
	u_int8 mode)
{
	ICANL2_SETACCMASK_PB pb;
	M_SG_BLOCK blk;

	pb.accMask	= accMask;
	pb.mode		= mode;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	return M_setstat( path, ICANL2_BLK_SETACCMASK, (INT32_OR_64)&blk );
}

/********************************* ICANL2API_CreateObject ********************
 *
 *  Description: Create a new message object
 *
 *  This routine creates a message object and return an object handle that
 *  must be passed to other ICANL2API_xxx() functions.
 *
 *	You must define all objects before enabling the CAN communication via
 *	ICANL2API_EnableCan()!
 *
 *  The following object types
 *  exist, specified with the <type> parameter:
 *
 *	- ICANL2_OBJT_GENERAL: general Rx message object:
 *	  if created, receives ALL objects on the bus regardless of its CAN id.
 *	  (tMode must be ICANL2_MOD_RECEIVE)
 *
 *	- ICANL2_OBJT_GLOBAL: global Rx message object:
 *	  if created, receives all message objects that are not received by
 *	  the standard Rx objects. (tMode must be ICANL2_MOD_RECEIVE)
 *
 *  - ICANL2_OBJT_RTR: this object transmits its CAN message
 *	  in response to a CAN RTR frame only (tMode must be ICANL2_MOD_TRANSMIT)
 *	  10 such objects are available.
 *
 *	- ICANL2_OBJT_STD: Standard Rx or Tx objects (max. 2048 objects avail.)
 *	  (distinguished by <tMode>):
 *
 *		- ICANL2_MOD_RECEIVE: 	this object can receive CAN frames
 *		- ICANL2_MOD_TRANSMIT: 	this object can transmit CAN frames
 *
 *  FIFOs
 *  -----
 *
 *  FIFOs for Rx objects:
 *
 *  Rx objects may or may not have a fifo. <fifoSize> specifies the number
 *  of CAN frames the fifo can buffer (0 means no fifo, max. size is 32766).
 *  If a fifo is to be created, an event can be generated
 *  when the number of fifo entries equals the number of entries specified by
 *  <sigLvl>.
 *  Another event is generated when the fifo is full (and CAN messages were
 *  lost).
 *  Data from the fifo must be read with ICANL2API_ReadFifo().
 *  The most recent CAN frame that was received can always be read using
 *  ICANL2API_NewestMsg().
 *
 *  FIFOs for Tx objects:
 *
 *	Tx objects MUST have a fifo. Again, <fifoSize> specifies the number
 *  of objects. Whether an event is generated after the frame has been sent can
 *  be specified on a per-frame basis.
 *	Data can be written to the FIFO using ICANL2API_WriteFifo().
 *
 *  Objects of type ICANL2_OBJT_RTR:
 *	These objects cannot have a fifo. Data for these objects must be
 * 	set using ICANL2API_WriteObject()
 *
 *	Events:
 *  --------
 *  Per-Object signals can be generated for the following events:
 *	- Rx: signal level reached
 *	- Rx: fifo overrun
 *	- Tx: tx complete (only for standard and OOB Tx objects, not for RTR)
 *
 *  Events are stored in a seperate event fifo within the driver. Each new
 *  entry in the fifo can generate a signal that is sent from the driver
 *  to the application. Events can be read from the application
 *  using ICANL2API_GetEvent()
 *
 *  Note:
 *  After startup there is already one special object created by the firmware:
 *  The out-of-band object with an objectHandle==ICANL2_OOB_HANDLE (3).
 *  Using this object, you can transmit high-priority objects bypassing
 *  the normal fifos. This object has fifo size of 127. Data must be written
 *  to the OOB object using ICANL2API_WriteFifo(). This object can also
 *  be used to send frames with arbitrary IDs without creating a special
 *  object for that ID.
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 id			can identifier (for standard objects only)
 *				 type		ICANL2_OBJT_xxx (see text above)
 *				 tMode		ICANL2_MOD_xxx (see text above)
 *               fifoSize   size of fifo (see text above)
 *				 sigLvl		fifo threshold to generate event/signal
 *  Output.....: returns:	>=0 object handle (0..2067) or
 *							-1 on error (errno set)
 *
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_CreateObject(
	MDIS_PATH path,
	u_int32 id,
	u_int32 type,
	u_int32 tMode,
	u_int32 fifoSize,
	u_int32 sigLvl)
{
	ICANL2_CREATEOBJECT_PB pb;
	M_SG_BLOCK blk;
	int32 rv;

	pb.id		= id;
	pb.type		= type;
	pb.tMode	= tMode;
	pb.fifoSize	= fifoSize;
	pb.sigLvl	= sigLvl;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	rv = M_getstat( path, ICANL2_BLK_CREATEOBJECT, (int32 *)&blk );

	if( rv < 0 )
		return -1;
	else
		return pb.objHdl;
}



/********************************* ICANL2API_DeleteObject ********************
 *
 *  Description: Delete a message object
 *
 *  This routine deletes a message object previously created by
 *  ICANL2API_CreateObject().
 *
 *	Objects can be deleted only while can communication is diabled, see
 *	ICANL2API_EnableCan()!
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl		object handle returned by ICANL2API_CreateObject()
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_DeleteObject(
	MDIS_PATH path,
	u_int32 objHdl)
{
	return M_setstat( path, ICANL2_DELETEOBJECT, (INT32_OR_64)objHdl );
}

/********************************* ICANL2API_EnableCan ***********************
 *
 *  Description: Enable or disable CAN communication
 *
 *	This routine effectively enables or disables the CAN chip to
 *  participate on the bus. It is also used to re-enable CAN communication
 *  after a BusOff.
 *
 *  Note that before enabling CAN communication, you must set
 * 	the bus parameters (ICANL2API_SetTiming()) and create all objects
 *  (ICANL2API_CreateObject())
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 enable		1=enable 0=disable
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_EnableCan(
	MDIS_PATH path,
	u_int8 enable)
{
	return M_setstat( path, ICANL2_ENABLECAN, (INT32_OR_64)enable );
}

/********************************* ICANL2API_ReadFifo ************************
 *
 *  Description: Read CAN frames from an object's fifo
 *
 *	This routine reads a maximum of <maxFrames> CAN frames from an Rx
 *  object's fifo. This routine is always non-blocking. If not enough
 *  frames in fifo it copies the number of entries currently present
 *  in the fifo.
 *
 *  Structure of each fifo entry:
 *		data_len: Length of CAN frame in bytes
 *		tx_flags: not applicable
 *		data	: CAN frame data
 *		id		: CAN id of received object (only global and general objects,
 *				  invalid for standard objects)
 *		time	: timestamp of frame (in units of 5.33 us)
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl	    object handle returned by ICANL2API_CreateObject()
 *				 msgBuf		array of ICANL2_DATA messages where
 *							the CAN frames will be stored
 *				 maxFrames	maximum number of frames to read from object
 *  Output.....: returns	>=0 number of frames read from fifo or
 *							-1 on error (errno set)
 *  Globals....: - (modifies current MDIS channel number)
 ****************************************************************************/
int32 __MAPILIB ICANL2API_ReadFifo(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf,
	u_int32 maxFrames)
{
	int32 length;

	/* set the current channel to the objHdl */
	if( M_setstat( path, M_MK_CH_CURRENT, (INT32_OR_64)objHdl ))
		return -1;

	length = M_getblock( path, (u_int8 *)msgBuf, maxFrames *
						 sizeof(ICANL2_DATA));
	if( length < 0 )
		return -1;
	return length / sizeof(ICANL2_DATA);
}

/********************************* ICANL2API_WriteFifo ***********************
 *
 *  Description: Write a number of CAN frames to an objects's fifo
 *
 *	This routine send the specified number of CAN frames to a Tx object's
 *  fifo. This routine is always non-blocking.
 *  If not enough space left in FIFO it places only those entries into
 *  the fifo that fit into the fifo.
 *
 *  Structure of each fifo entry:
 *		data_len: Length of CAN frame in bytes
 *		tx_flags: ICANL2_FE_GENINT: Generate tx complete event when sent
 *				  ICANL2_FE_REMOTE: Send the frame as an RTR frame (Note 1)
 *		data	: CAN frame data
 *		id		: CAN id of tx object (only when used when sending to
 *				  the out-of-band (OOB) object)
 *		time	: ignored
 *
 *  Note 1) A seperate rx object is required for the RTR answer
 *
 *  Note that the Tx objects have an implicit priority: The object with the
 *  lowest CAN-ID has the highest priority
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl	    object handle returned by ICANL2API_CreateObject()
 *				 msgBuf		array of ICANL2_DATA messages with
 *							the CAN frames to send
 *				 nFrames	number of frames to send to object
 *  Output.....: returns	>=0 number of frames sent to fifo
 *							-1 on error (errno set)
 *  Globals....: - (modifies current MDIS channel number)
 ****************************************************************************/
int32 __MAPILIB ICANL2API_WriteFifo(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf,
	u_int32 nFrames)
{
	int32 length;

	/* set the current channel to the objHdl */
	if( M_setstat( path, M_MK_CH_CURRENT, (INT32_OR_64)objHdl ))
		return -1;

	length = M_setblock( path, (u_int8 *)msgBuf, nFrames *
						 sizeof(ICANL2_DATA));
	if( length < 0 )
		return -1;
	return length / sizeof(ICANL2_DATA);
}

/********************************* ICANL2API_NewestMsg ***********************
 *
 *  Description: Get the most recently received CAN frame from an Rx object
 *
 *	This routine reads the most recently received CAN frame from the message
 *  object. When no new message has arrived since initialisation or last
 *  call to ICANL2API_NewestMsg(), this function returns 1 to tell that
 *  there was no new data. (msgBuf invalid in this case)
 *
 *  The following fields of msgBuf will be set:
 *
 *		data_len: Length of CAN frame in bytes
 *		tx_flags: not applicable
 *		data	: CAN frame data
 *		id		: CAN id of received object
 *		time	: invalid
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl	    object handle returned by ICANL2API_CreateObject()
 *				 msgBuf		pointer to buffer where
 *							the CAN frame will be stored
 *  Output.....: returns:	0=ok
 *							1=no new data arrived
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_NewestMsg(
	MDIS_PATH path,
	u_int32 objHdl,
	ICANL2_DATA *msgBuf)
{
	ICANL2_NEWESTMSG_PB pb;
	M_SG_BLOCK blk;
	int32 rv;

	pb.objHdl	= objHdl;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	rv = M_getstat( path, ICANL2_BLK_NEWESTMSG, (int32 *)&blk );
	if( rv == 0 ){
		if( pb.result == 0 )
			*msgBuf = pb.msg;			/* copy newest data block to caller */
		return pb.result;
	}
	return rv;
}

/**************************** ICANL2API_FifoInfo ****************************
 *
 *  Description: Get the number of used entries of an object's fifo.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl	    object handle returned by ICANL2API_CreateObject()
 *  Output.....: numEntries number entries
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_FifoInfo(
	MDIS_PATH path,
	u_int32 objHdl,
	int32 *numEntries )
{
	ICANL2_FIFOINFO_PB pb;
	M_SG_BLOCK blk;
	int32 rv;

	pb.objHdl	= objHdl;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	rv = M_getstat( path, ICANL2_BLK_FIFOINFO, (int32 *)&blk );
	if( rv == 0 ){
		*numEntries = pb.numEntries;
	}

	return rv;
}

/***************************** ICANL2API_FwIdent *****************************
 *
 *  Description: Returns revision of module firmware.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *  Output.....: returns:	0=ok
 *               ver        fw revision coded as 32bit value
 *                          e.g. 0x00010203 = Ver.1.02.03
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_FwIdent(
	MDIS_PATH path,
	int32 *ver )
{
	int32 rv, v;

	rv = M_getstat( path, ICANL2_FWIDENT, &v );

	if( rv == 0 )
		*ver = v;

	return rv;
}

/***************************** ICANL2API_FwInfo *****************************
 *
 *  Description: Get some status information from firmware.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *  Output.....: returns:	0=ok
 *               numEvents  number entries in event fifo
 *               tick       module time in steps of 5.33us
 *               freeMem    free memory in bytes
 *               numCyc     number of cyclic objects
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_FwInfo(
	MDIS_PATH path,
	int32 *numEvents,
	int32 *tick,
	int32 *freeMem,
	int32 *numCyc )
{
	ICANL2_FWINFO_PB pb;
	M_SG_BLOCK blk;
	int32 rv;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	rv = M_getstat( path, ICANL2_BLK_FWINFO, (int32 *)&blk );
	if( rv == 0 ) {
		*numEvents = pb.numEvents;
		*tick      = pb.tick;
		*freeMem   = pb.freeMem;
		*numCyc    = pb.numCyc;
	}

	return rv;
}

/********************************* ICANL2API_GetEvent ************************
 *
 *  Description: Read event from the event fifo
 *
 *	This routine reads the oldest event occurred out from the event fifo.
 *  It can be blocking or non-blocking:
 *
 *	timeout=-1	wait forever until event arrives
 *	timeout=0	do not block, return immediately
 *	timeout>=0	timeout in ms to wait for event
 *
 * 	Several events are generated by the firmware. Some are global while
 *  others belong to a specific object.
 *
 *  Here is a list of the possible events:
 *
 *  Event code 					 Type Description
 *	---------------------------- ---- ---------------------------
 *  ICANL2_EV_NONE			0x00  -	  no event present
 *  ICANL2_EV_SIGLVL		0x01  O	  fifo reached signal level threshold 1)
 *  ICANL2_EV_BOFF			0x02  G	  i82527 in BusOff status
 *  ICANL2_EV_MSG_LOST		0x03  G	  Message lost (firmware overload)
 *  ICANL2_EV_WARN			0x04  G   CAN controller in warning state
 *  ICANL2_EV_FIFO_FULL		0x05  O	  Messages lost due to fifo full. 1)
 *  ICANL2_EV_TX_COMPLETE	0x07  O	  Frame transmission complete.
 *  ICANL2_EV_STUFF_ERROR	0x11  G   i82527 stuff error
 *  ICANL2_EV_FORM_ERROR	0x12  G	  i82527 msg malformed
 *  ICANL2_EV_ACK_ERROR		0x13  G	  i82527 msg not acknowledged
 *  ICANL2_EV_BIT1_ERROR	0x14  G	  i82527 CAN bus stuck to 1
 *  ICANL2_EV_BIT0_ERROR	0x15  G	  i82527 CAN bus stuck to 0
 *  ICANL2_EV_CRC_ERROR		0x16  G	  i82527 msg wrong crc
 *  ICANL2_EV_WAKE			0x20  G	  i82527 wake up
 *
 *  Type: G - global event (*objHdlP not valid)
 *    	  O - object related event (*objHdlP contains the object handle)
 *
 *  Note 1) ICANL2_EV_SIGLVL and ICANL2_EV_FIFO_FULL are sent only once
 *  when the condition is reached. Further incoming frames will not send
 *  the event again. The condition is cleared by reading out the fifo.
 *
 *  Another way to get event notification is to send a signal, see
 *  ICANL2API_Signal()
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 timeout	timeout to wait for event (see text above)
 *  Output.....: returns	0  = no event occurred
 *							>0 = event number
 *							-1 = error (errno set)
 *				 *objHdlP	contains the object handle for object related
 *							events
 * 				 *timeStampP timestamp for that event (5.33us resolution)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_GetEvent(
	MDIS_PATH path,
	int32 timeout,
	u_int32 *objHdlP,
	u_int32 *timeStampP)
{
	ICANL2_GETEVENT_PB pb;
	M_SG_BLOCK blk;
	int32 rv;

	pb.timeout	= timeout;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	rv = M_getstat( path, ICANL2_BLK_GETEVENT, (int32 *)&blk );

	if( rv < 0 )
		return -1;
	else {
		*objHdlP 	= pb.objHdl;
		*timeStampP = pb.timeStamp;
		return pb.event;
	}
}

/****************************** ICANL2API_Signal *****************************
 *
 *  Description: Install or deinstall a signal
 *
 *  This routine installs or removes a signal that is sent when a new event
 *  arrives in the event fifo, see ICANL2API_GetEvent().
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *               signal     number of signal to be installed
 *                          0 deinstalls a previously installed signal
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_Signal(
	MDIS_PATH path,
	int32 signal)
{
	if( signal == 0 ) {
		return M_setstat( path, ICANL2_CLRSIG_EVENT, 0  );
	} else {
		return M_setstat( path, ICANL2_SETSIG_EVENT, (INT32_OR_64)signal  );
	}
}

/********************************* ICANL2API_WriteObject *********************
 *
 *  Description: Send a single frame to a CAN Tx object (RTR objects)
 *
 *	This function sends a single CAN frame to the specified Tx object. This
 *  function must be used to update the data for a remote (RTR) Tx object.
 *  It can be used for any other Tx object as well, but for normal Tx objects
 *  it is preferrable to use ICANL2API_WriteFifo() due to its higher speed.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl		object handle for Tx object
 *				 dataLen	number of bytes in CAN frame
 *				 data		the CAN frame to send [0..8 bytes]
 *				 flags		transmission flags:
 *							ICANL2_FE_GENINT:
 *							 send tx complete event when frame transmitted
 *							 (ignored for RTR objects)
 *							ICANL2_FE_REMOTE
 *							 send this frame as a remote transmit request.
 *							 (dataLen must be 0)
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_WriteObject(
	MDIS_PATH path,
	u_int32 objHdl,
	u_int8	dataLen,
	const u_int8  *data,
	u_int16	flags)
{
	ICANL2_WRITEOBJECT_PB pb;
	M_SG_BLOCK blk;
	u_int8 *dp = pb.data;

	pb.objHdl	= objHdl;
	pb.dataLen	= dataLen;
	pb.flags	= flags;

	while( dataLen-- )
		*dp++ = *data++;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	return M_setstat( path, ICANL2_BLK_WRITEOBJECT, (INT32_OR_64)&blk );
}

/********************************* ICANL2_SendCyclic *************************
 *
 *  Description: Cyclic transmission of Tx object
 *
 *	This function starts to send a Tx object cyclically. The data that
 *  is passed via this function is sent periodically over the bus. To
 *  send different data, call this function again.
 *
 *  To stop the periodical sending of that object, call this function with
 *  a cycle time of zero.
 *
 *  The object must have been created before using ICANL2API_CreateObject()
 *	and the timer must have been initialized using ICANL2API_SetTimer()/
 *  ICANL2API_StartTimer()
 *
 *  Note that cyclic objects cannot have a tx fifo
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 objHdl		object handle for Tx object
 *				 dataLen	number of bytes in CAN frame
 *				 data		the CAN frame to send [0..8 bytes]
 *				 cycleTime	cycle time in units of timer ticks
 *							(as defined by ICANL2API_SetTimer())
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_SendCyclic(
	MDIS_PATH path,
	u_int32 objHdl,
	u_int8	dataLen,
	const u_int8  *data,
	u_int32	cycleTime)
{
	ICANL2_SENDCYCLIC_PB pb;
	M_SG_BLOCK blk;
	u_int8 *dp = pb.data;

	pb.objHdl	= objHdl;
	pb.dataLen	= dataLen;
	pb.cycleTime= cycleTime;

	while( dataLen-- )
		*dp++ = *data++;

	blk.size = sizeof(pb);
	blk.data = (void *)&pb;

	return M_setstat( path, ICANL2_BLK_SENDCYCLIC, (INT32_OR_64)&blk );
}

/********************************* ICANL2API_SetTimer ************************
 *
 *  Description: Set ICANL2 firmware internal timer tick resolution
 *
 *	This tick is used for cyclic sending of frames, see ICANL2API_SendCyclic()
 *	To activate this timer, use ICANL2API_StartTimer()
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 tickIv		tick interval in units of 8us
 *							(real resolution is 5.33us)
 *							maximum tickIv can be 43690 (349ms)
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_SetTimer(
	MDIS_PATH path,
	u_int32 tickIv)
{
	return M_setstat( path, ICANL2_SETTIMER, (INT32_OR_64)tickIv );
}

/********************************* ICANL2API_StartTimer **********************
 *
 *  Description: Start/Stop ICANL2 firmware internal timer
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *				 on			0=off 1=on
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_StartTimer(
	MDIS_PATH path,
	int32 on)
{
	return M_setstat( path, ICANL2_STARTTIMER, (INT32_OR_64)on );
}

/********************************* ICANL2API_SyncTimer **********************
 *
 *  Description: Syncronize ICANL2 firmware internal timer
 *
 *	Reset the tick timer so that the next tick appears exactly after the
 *  tick interval
 *---------------------------------------------------------------------------
 *  Input......: path		MDIS path number for device
 *  Output.....: returns:	0=ok
 *							-1 on error (errno set)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB ICANL2API_SyncTimer(
	MDIS_PATH path)
{
	return M_setstat( path, ICANL2_SYNCTIMER, 0 );
}





