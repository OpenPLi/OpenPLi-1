#ifndef __DVB_CAHANDLER_H_
#define __DVB_CAHANDLER_H_

#include <lib/base/eptrlist.h>
#include <lib/dvb/dvbservice.h>
#include <lib/socket/serversocket.h>

/*
 * eDVBCAHandler provides external clients with CAPMT objects
 *
 * The traditional way of receiving this information was by providing a listening
 * socket on /tmp/camd.socket.
 * For every channel change, a connection will be opened, and a CAPMT object is transmitted.
 *
 * This has a few disadvantages:
 * 1. a new connection has to be opened for each channel change
 * 2. only one external client can receive CAPMT objects
 * 3. when the client restarts, it has no way of requesting the DVBCAHandler
 * to reconnect
 *
 * To overcome these disadvantages, a new method has been added;
 * eDVBCAHandler now also provides a serversocket on "/tmp/.listen.camd.socket".
 * Clients can connect to this socket, and receive CAPMT objects as channel
 * changes occur. The socket should be left open.
 * Clients should check the ca_pmt_list_management field in the CAPMT objects, to
 * determine whether an object is the first or last object in the list, an object in the middle,
 * or perhaps an update for an existing service.
 *
 * the DVBCAHandler will immediately (re)transmit the current list of CAPMT objects when
 * the client (re)connects.
 *
 */

/* CAPMT client sockets */
#define PMT_SERVER_SOCKET "/tmp/.listen.camd.socket"
#define PMT_CLIENT_SOCKET "/tmp/camd.socket"

 /* ca_pmt_list_management values: */

#define LIST_MORE 0x00
												/* CA application should append a 'MORE' CAPMT object the list,
												 * and start receiving the next object
												 */
#define LIST_FIRST 0x01
												/* CA application should clear the list when a 'FIRST' CAPMT object
												 * is received, and start receiving the next object
												 */
#define LIST_LAST 0x02
												/* CA application should append a 'LAST' CAPMT object to the list,
												 * and start working with the list
												 */
#define LIST_ONLY 0x03
												/* CA application should clear the list when an 'ONLY' CAPMT object
												 * is received, and start working with the object
												 */
#define LIST_ADD 0x04
												/* CA application should append an 'ADD' CAPMT object to the current list,
												 * and start working with the updated list
												 */
#define LIST_UPDATE 0x05
												/* CA application should replace an entry in the list with an
												 * 'UPDATE' CAPMT object, and start working with the updated list
												 */

/* ca_pmt_cmd_id's: */
#define CMD_OK_DESCRAMBLING 0x01
												/* CA application should start descrambling the service in this CAPMT object,
												 * as soon as the list of CAPMT objects is complete
												 */
#define CMD_OK_MMI					0x02
#define CMD_QUERY						0x03
#define CMD_NOT_SELECTED		0x04
												/* CA application should stop descrambling this service
												 * (used when the last service in a list has left, note
												 * that there is no CI definition to send an empty list)
												 */

class eDVBCAHandler;

class ePMTClient : public eUnixDomainSocket
{
	unsigned char receivedTag[4];
	int receivedLength;
	unsigned char *receivedValue;
	char *displayText;
protected:
	eDVBCAHandler *parent;
	void connectionLost();
	void dataAvailable();
	void clientTLVReceived(unsigned char *tag, int length, unsigned char *value);
	void parseTLVObjects(unsigned char *data, int size);
public:
	ePMTClient(eDVBCAHandler *handler, int socket);
};

class CAService: public eUnixDomainSocket
{
	int lastPMTVersion;
	eServiceReferenceDVB me;
	unsigned char *capmt;
	eTimer retry;
public:
	const eServiceReferenceDVB &getRef() const { return me; }
	void sendCAPMT();
	int writeCAPMTObject(eSocket *socket, int list_management = -1);
	CAService( const eServiceReferenceDVB &service );
	~CAService()
	{
		if (capmt) delete [] capmt;
	}
	void buildCAPMT( PMT *pmt );
	int getCAPMTVersion() { return lastPMTVersion; }
	void connectionLost();
};

static bool operator==( const CAService *caservice, const eServiceReferenceDVB &service )
{
	return caservice->getRef() == service;
}

class eDVBCAHandler: public eDVBCaPMTClient, public eServerSocket
{
	ePtrList<CAService> services;
	ePtrList<ePMTClient> clients;
	eTimer serviceLeft;

	void newConnection(int socket);
	void distributeCAPMT();
	void serviceGone();

	static eDVBCAHandler *instance;

public:
	void enterService( const eServiceReferenceDVB &service );
	void leaveService( const eServiceReferenceDVB &service );
	void handlePMT( const eServiceReferenceDVB &service, PMT *pmt );
	void connectionLost(ePMTClient *client);
	eDVBCAHandler();
	~eDVBCAHandler();

	static eDVBCAHandler *getInstance() { return instance; }
	Signal1<void, const char*> clientname;
	Signal1<void, const char*> clientinfo;
	Signal1<void, const char*> verboseinfo;
	Signal1<void, int> usedcaid;
	Signal1<void, int> decodetime;
	Signal1<void, const char*> usedcardid;
};

#endif // __DVB_CAHANDLER_H_
