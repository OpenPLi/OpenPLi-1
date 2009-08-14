/***************************************************************************
 *                                                                         *
 *   You are not allowed to change this file in any way when used          *
 *                               with LCARS                                *
 *                                                                         *
 ***************************************************************************/
/*
$Log: cam.h,v $
Revision 1.6  2002/11/16 02:34:55  obi
use only one ca file descriptor

Revision 1.5  2002/06/08 20:21:09  TheDOC
adding the cam-sources with slight interface-changes

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef CAM_H
#define CAM_H

#include "pmt.h"

class cam
{
	void sendCAM(void *command, unsigned int len);
	
	unsigned short ONID;
	unsigned short TS;
	unsigned short SID;
	unsigned short pid_count;
	unsigned short PIDs[10];
	unsigned short CAID;
	unsigned short ECM;
	unsigned short EMM;
	pmt_data pmt_entry;

	int camfd;
public:
	cam();
	~cam();

	void initialize();

	// These functions set the different PIDs
	void setONID(unsigned short PID) { ONID = PID; }
	void setTS(unsigned short PID) { TS = PID; }
	void setSID(unsigned short PID) { SID = PID; }
	void addPID(unsigned short PID) { PIDs[pid_count++] = PID; }
	void setCAID(unsigned short PID) { CAID = PID; }
	void setECM(unsigned short PID) { ECM = PID; }
	void setEMM(unsigned short PID) { EMM = PID; }
	void setPMTentry(pmt_data pmt) { pmt_entry = pmt; }

	void cam::cam_answer();

	bool isfree();

	// Returns the CAID - First the CAID has to be read by cam::readCAID()
	unsigned short getCAID() { return CAID; }

	// Reads the CAID from the card
	void readCAID();

	// Inits the CAM
	void init();

	// Inits the CAM
	void init2();

	// Resets the CAM
	void reset();

	// Starts the descambling
	void start();

	// Starts the EMM-parsing with the previously set EMM-PID
	void startEMM();

	// Descrambles a channel with the previously set PIDs
	void descramble();

};

#endif
