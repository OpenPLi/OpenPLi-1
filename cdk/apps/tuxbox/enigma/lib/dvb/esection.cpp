#include <lib/dvb/esection.h>
#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#include <lib/base/ebase.h>
#include <lib/base/eerror.h>
#include <lib/system/elock.h>
#include <lib/system/info.h>

#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#else
#include <linux/dvb/dmx.h>
#endif

static pthread_mutex_t slock=
	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
ePtrList<eSection> eSection::active;

eSectionReader::eSectionReader()
{
	handle=-1;
}

int eSectionReader::getHandle()
{
	return handle;
}

void eSectionReader::close()
{
	if (handle != -1)
		::close(handle);
	handle=-1;
}

int eSectionReader::open(int pid, __u8 *data, __u8 *mask, __u8 *mode, int len, int _flags, const char* dmxdev)
{
	tableid=data[0];
	tableid_mask=mask[0];
	flags=_flags;
#if HAVE_DVB_API_VERSION < 3
	dmxSctFilterParams secFilterParams;
#else
	dmx_sct_filter_params secFilterParams;
#endif

	close();

	handle=::open(dmxdev, O_RDWR|O_NONBLOCK);
//	eDebug("opened handle %d", handle);
	if (handle<0)
	{
		perror(dmxdev);
		return -errno;
	}
#if HAVE_DVB_API_VERSION == 3
	else
	{
		if (::ioctl(handle, DMX_SET_BUFFER_SIZE, 128*1024) < 0 )
			eDebug("DMX_SET_BUFFER_SIZE failed (%m)");
	}
#endif

	secFilterParams.pid=pid;

#if HAVE_DVB_API_VERSION < 3
	memset(secFilterParams.filter.filter, 0, DMX_FILTER_SIZE);
	memset(secFilterParams.filter.mask, 0, DMX_FILTER_SIZE);
#else
	memset(&secFilterParams.filter, 0, sizeof(struct dmx_filter));
#endif

	secFilterParams.timeout=0;
	secFilterParams.flags=DMX_IMMEDIATE_START;
#if HAVE_DVB_API_VERSION < 3
	if ( eSystemInfo::getInstance()->hasNegFilter() )
		secFilterParams.flags=0;
#endif
	if (flags&SECREAD_CRC)
		secFilterParams.flags|=DMX_CHECK_CRC;
	for (int i = 0; i < len; i++)
	{
		secFilterParams.filter.filter[i]=data[i];
		secFilterParams.filter.mask[i]=mask[i];
#if HAVE_DVB_API_VERSION >= 3
		secFilterParams.filter.mode[i]=mode[i];
#endif
	}

#ifdef ESECTION_DEBUG
	eDebugNoNewLine("%02x: ", pid);
	for (int i=0; i<DMX_FILTER_SIZE; i++)
		eDebugNoNewLine("%02x ", secFilterParams.filter.filter[i]);
	eDebugNoNewLine(" (napi)\n    ");
	for (int i=0; i<DMX_FILTER_SIZE; i++)
		eDebugNoNewLine("%02x ", secFilterParams.filter.mask[i]);
	eDebug("");
#endif

	if (ioctl(handle, DMX_SET_FILTER, &secFilterParams) < 0)
	{	
		perror("DMX_SET_FILTER\n");
		::close(handle);
		return -1;
	}

#if HAVE_DVB_API_VERSION < 3
	if ( eSystemInfo::getInstance()->hasNegFilter() )
	{
		__u8 negfilter[DMX_FILTER_SIZE];
		memset(negfilter, 0, DMX_FILTER_SIZE);
		memcpy(negfilter, mode, len);
		if (::ioctl(handle, DMX_SET_NEGFILTER_MASK, negfilter) < 0)
			eDebug("DMX_SET_NEGFILTER_MASK (%m)");
		if (::ioctl(handle, DMX_START, 0) < 0)
			eDebug("DMX_START failed(%m)");
	}
#endif

	return 0;
}

int eSectionReader::read(__u8 *buf)
{
	int rd = ::read(handle, buf, 4098);
	if (rd < 0)
	{
		/* backup errno, for instance eDebug could change it */
		int error = errno;
		if (error == EAGAIN)
			return error;
		eDebug("section read(%m)");
		return error;
	}
	if (rd > 0 && (buf[0] & tableid_mask) != (tableid & tableid_mask))
	{
		eDebug("skip section data.. table_id isn't valid");
		return -2;
	}
#if 0
	ASSERT(rd <= 4098);
	if ( rd < 3 )
	{
		eFatal("read less then 3 bytes from section.. skip");
		return -1;
	}
	int seclen=(buf[1]&0x0F)<<8;
	seclen|=buf[2]&0xFF;
	seclen+=3;
	if ( rd < seclen )
	{
		eFatal("cannot read the complete section(%d/%d)",
			rd, seclen);
		return -1;
	}
	if ( rd > seclen )
	{
		eFatal("read more bytes than section length(%d/%d)",
			rd, seclen);
		return -1;
	}
#endif
	return 0;
}

eSection::eSection(int _pid, int _tableid, int _tableidext, int _version, int _flags, int _tableidmask)
	:context(eApp), notifier(0), pid(_pid), tableid(_tableid)
	,tableidext(_tableidext), tableidmask(_tableidmask), maxsec(0)
	,section(-1), flags(_flags), timer(0)
	,lockcount(0), version(_version)
{
	setContext(context);
}

eSection::eSection()
	:context(eApp), notifier(0), pid(0), tableid(0), tableidext(0)
	,tableidmask(0), maxsec(0), section(-1), flags(0)
	,timer(0), lockcount(0), version(0)
{
	setContext(context);
}

eSection::~eSection()
{
	closeFilter();
	if (lockcount)
		eDebug("deleted still locked table");
	delete timer;
	timer=0;
}

int eSection::start( const char* dmxdev )
{
	if (timer && version==-1)
	{
		timer->start(
			pid == 0x14 /* TOT/TDT */ ? 90000 :
			pid == 0x10     /* NIT */ ? 12000 :
			pid == 0x00     /* PAT */ ?  4000 :
			pid == 0x11     /* SDT */ ? 60000 : /* DISH/BEV needs 60s instead of 5 */
			pid == 0x12     /* EIT */ ? 15000 :
			tableid == 0x02 /* PMT */ ?  4000 : 10000, true);
	}
	else if (timer)
	{
		/* make sure we delete the timer, so we won't restart it later on */
		delete timer;
		timer = NULL;
	}
	return setFilter(pid, tableid, tableidext, version, flags, tableidmask, dmxdev);
}

int eSection::setFilter(int pid, int tableid, int tableidext, int version, int flags, int tableidmask, const char *dmxdev)
{
	this->pid = pid;
	this->tableid = tableid,
	this->tableidext = tableidext;
	this->version = version;
	this->flags = flags;
	this->tableidmask = tableidmask;
	this->section = -1;
	this->maxsec = 0;
	closeFilter();
	__u8 data[4], mask[4], mode[4];
	memset(mode,0,4);
	data[0]=tableid; mask[0]=tableidmask;
	data[1]=0; mask[1]=0;
	data[2]=0; mask[2]=0;
	data[3]=0; mask[3]=0;
	if (tableidext!=-1)
	{
		data[1]=tableidext>>8; mask[1]=0xFF;
		data[2]=tableidext&0xFF; mask[2]=0xFF;
	} 
	if (version!=-1)
	{
		if ( eSystemInfo::getInstance()->hasNegFilter() )
		{
			data[3]=version<<1; mask[3]=0x3E; mode[3]=0x3E;
		}
		else
		{
			data[3]=version; mask[3]=0xFF;
		}
	}

	reader.open(pid, data, mask, mode, 4, flags, dmxdev);
	if (reader.getHandle() < 0)
		return -ENOENT;

	if (!(flags&SECREAD_NOABORT))
	{
		singleLock s(slock);
		active.push_back(this);
	}

	if ( timer && (flags&SECREAD_NOTIMEOUT) )
	{
		delete timer;
		timer=0;
	}

	if (notifier)
		delete notifier;

	notifier=new eSocketNotifier(context, reader.getHandle(), eSocketNotifier::Read);

	CONNECT(notifier->activated, eSection::data);

	return 0;
}

void eSection::closeFilter()
{
	if (reader.getHandle() >= 0)
	{
		if (!(flags&SECREAD_NOABORT))
		{
			singleLock s(slock);
			active.remove(this);
		}
		delete notifier;
		notifier=0;
		if (timer)
			timer->stop();
		reader.close();
	}
}

void eSection::data(int socket)
{
	(void)socket;
	
	while (1)
	{
		if (lockcount)
			eDebug("eSection::data on locked section!");

		int ret = reader.read(buf);

		switch(ret)
		{
			case 0:
				if (section == -1)
				{
					section=0;
					if (timer)
						timer->restart();
				}
				break;
			default:
				eDebug("section read error %d for pid %04x, tid %02x, tidmask %02x, tidext %04x... restart timeout\n",
					ret, pid, tableid, tableidmask, tableidext);
			case EAGAIN:
			case -2: // internal error table id not correct
				if (timer)
					timer->restart();
				return;
		}

		maxsec=buf[7];

		//  printf("%d/%d, we want %d  | service_id %04x | version %04x\n", buf[6], maxsec, section, (buf[3]<<8)|buf[4], buf[5]);

		if (flags&SECREAD_INORDER)
		{
			if (section != buf[6])
				break;
			++section;
		} 

		version=buf[5];

		// get new valid data restart timeout
		int err=sectionRead(buf);
		if (err)
		{
			if (err>0)
				err=0;   
			closeFilter();      
			sectionFinish(err);
			return;
		}

		if ( flags&SECREAD_INORDER && section > maxsec )
			// last section?
		{
			closeFilter(); // stop feeding
			sectionFinish(0);
			break;
		}

		// when more data to read.. restart timeout..
		if ( timer )
			timer->restart();
	}
}

void eSection::timeout()
{
	eDebug("Section timeout PID %04x", pid);
	closeFilter();
	sectionFinish(-ETIMEDOUT);
}

int eSection::abort()
{
	if (reader.getHandle() >= 0)
	{
		closeFilter();
		sectionFinish(-ECANCELED);
		return 0;
	} else
		return -ENOENT;
}

int eSection::abortAll()
{
	singleLock s(slock);
	for (ePtrList<eSection>::iterator it(active.begin()); it != active.end();)
	{
		// abort only sections in main context..
		if ( it->context == eApp )
		{
			eSection *sec = *it;
			++it;
			sec->abort();
		}
		else
			++it;
	}
	return 0;
}

int eSection::sectionRead(__u8 *data)
{
	(void)data;
	return -1;
}

void eSection::sectionFinish(int)
{
}

int eSection::lock()
{
	return ++lockcount;
}

int eSection::unlock()
{
	if (lockcount)
		return lockcount--;
	else
		eDebug("unlocking... but not locked");
	return 0;
}

void eSection::setContext( eMainloop *context )
{
	this->context = context;
	delete timer;
	timer=0;
	if ( !(flags&SECREAD_NOTIMEOUT) )
	{
		timer=new eTimer(context);
		CONNECT(timer->timeout, eSection::timeout);
	}
	if ( notifier )
		eWarning("setContext with running notifier !!!");
}

void eTable::sectionFinish(int err)
{
	if (err)
		error=err;
	ready=1;
	/*emit*/ tableReady(error);
}

eTable::eTable(int pid, int tableid, int tableidext, int version)
	:eSection(pid, tableid, tableidext, version
	,(pid==0x14)?0:(SECREAD_INORDER|SECREAD_CRC), 0xFF)
	,error(0), ready(0)
{
}

eTable::eTable(int pid, int tableid, int tableidmask, int tableidext, int version)
	:eSection(pid, tableid, tableidext, version
	,(pid==0x14)?0:(SECREAD_INORDER|SECREAD_CRC), tableidmask)
	,error(0), ready(0)
{
}

eTable::eTable()
	:error(0), ready(0)
{
}

eTable *eTable::createNext()
{
	return 0;
}

void eAUGTable::slotTableReady(int error)
{
	getNext(error);
}
