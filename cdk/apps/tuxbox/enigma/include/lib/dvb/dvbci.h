#ifndef __core_dvb_ci_h
#define __core_dvb_ci_h

#include <lib/dvb/service.h>
#include <lib/base/ebase.h>
#include <lib/base/thread.h>
#include <lib/base/message.h>
#include <lib/system/elock.h>
#include <set>
#include <queue>

#define PMT_ENTRYS  256

struct session_struct
{
	unsigned int tc_id;
	unsigned long service_class;
	unsigned int state;
	unsigned int internal_state;
};

struct tempPMT_t
{
	int type;  // 0 = version 1=pid 2=descriptor
	unsigned char *descriptor;
	union
	{
		unsigned short pid;
		unsigned short version;
	};
	unsigned short streamtype;
};

#define MAXTRANSPORTSESSIONS 32
#define PCMCIABUFLEN 255
#define LPDUHEADERLEN 2
#define LPDUPAYLOADLEN (PCMCIABUFLEN - LPDUHEADERLEN)

typedef struct _lpduQueueElem lpduQueueElem;
typedef struct _lpduQueueElem * ptrlpduQueueElem;

typedef struct _lpduQueueHeader lpduQueueHeader;

struct queueData
{
	__u8 prio;
	unsigned char tc_id;
	unsigned char *data;
	unsigned int len;
	queueData( unsigned char tc_id, unsigned char *data, unsigned int len, __u8 prio = 0 )
		:prio(prio), tc_id(tc_id), data(data), len(len)
	{

	}
	bool operator < ( const struct queueData &a ) const
	{
		return prio < a.prio;
	}
};

struct _lpduQueueElem
{
	unsigned char lpduLen;
	unsigned char lpdu[PCMCIABUFLEN];
	ptrlpduQueueElem nextElem;
};
			
struct _lpduQueueHeader
{
	long numLPDUS;
	ptrlpduQueueElem firstLPDU;
};

#define CIServiceMap std::map<int, std::list<tempPMT_t> >

class eDVBCI: private eThread, public eMainloop, public Object
{
	static int instance_count;
	std::priority_queue<queueData> sendqueue;
	bool newTransponder;
	void init_eDVBCI();
protected:
	enum
	{
		stateInit, stateError, statePlaying, statePause
	};
	int state;
	int fd;
	eSocketNotifier *ci;

	int ci_state;
	int buffersize;	

	eTimer pollTimer;

	int tempPMTentrys;

	CIServiceMap services;
	std::map<int,int> versions;

	struct session_struct sessions[32];

	char appName[256];
	unsigned short caids[256];
	unsigned int caidcount;

	unsigned char ml_buffer[1024];
	int ml_bufferlen;
	int ml_buffersize;

	//----------------------
	lpduQueueHeader lpduReceiveQueues[MAXTRANSPORTSESSIONS];

	ptrlpduQueueElem eDVBCI::AllocLpduQueueElem(unsigned char t_c_id);
	int eDVBCI::lpduQueueElemIsMore(ptrlpduQueueElem curElem);

	//----------------------

	void clearCAIDs();
	void addCAID(int caid);	
	void pushCAIDs();	
	void addService(int program);
	void PMTflush(int program);
	void PMTaddPID(int sid, int pid,int streamtype);
	void PMTaddDescriptor(int sid, unsigned char *data);
	void PMTsetVersion(int sid, int version);
	void newService();
	void create_sessionobject(unsigned char *tag,unsigned char *data,unsigned int len,int session);

	bool sendData(unsigned char tc_id,unsigned char *data,unsigned int len);	
	void sendTPDU(unsigned char tpdu_tag,unsigned int len,unsigned char tc_id,unsigned char *data,bool dontQueue=false);
	void help_manager(unsigned int session);
	void app_manager(unsigned int session);
	void ca_manager(unsigned int session);

	void handle_session(unsigned char *data,int len);
	int service_available(unsigned long service_class);
	void handle_spdu(unsigned int tpdu_tc_id,unsigned char *data,int len);	
	void receiveTPDU(unsigned char tpdu_tag,unsigned int len,unsigned char tpdu_tc_id,unsigned char *data);
	void incoming(unsigned char *buffer,int len);
	void dataAvailable(int what);
	void poll();
	void startTimer();
	void stopTimer();
	void updateCIinfo(unsigned char *buffer);

	void mmi_begin();
	void mmi_end();
	void mmi_enqansw(unsigned char *answ);
	void mmi_menuansw(int);

public:
	struct eDVBCIMessage
	{
		enum
		{
			start,
			reset, 
			init,
			exit,
			go,
			PMTsetVersion,
			PMTflush,
			PMTaddPID,
			PMTaddDescriptor,
			mmi_begin,
			mmi_end,
			mmi_enqansw,
			mmi_menuansw,
			getcaids,
			enable_ts,
			disable_ts,
			getAppName
		};
		int type;
		int sid;
		unsigned char *data;
		int pid;
		int streamtype;

		eDVBCIMessage() { }
		eDVBCIMessage(int type): type(type) { }
		eDVBCIMessage(int type, int sid, unsigned char *data): type(type),sid(sid),data(data) { }
		eDVBCIMessage(int type, int sid): type(type),sid(sid) { }
		eDVBCIMessage(int type, int sid, int pid, int streamtype): type(type),sid(sid),pid(pid),streamtype(streamtype) { }

	};
	eFixedMessagePump<eDVBCIMessage> messages;
	
	void gotMessage(const eDVBCIMessage &message);

	eDVBCI();
	~eDVBCI();
	
	void thread();
	Signal1<void, const char*> ci_progress;
	Signal2<void, const char*, int> ci_mmi_progress;
};
#endif
