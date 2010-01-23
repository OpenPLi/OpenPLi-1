#ifndef DISABLE_FILE
#include <lib/dvb/record.h>
#include <config.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/servicedvb.h>
#include <lib/system/file_eraser.h>
#include <lib/system/econfig.h>
#include <signal.h>

#ifndef DMX_LOW_BITRATE
#define DMX_LOW_BITRATE 0x4000
#endif

#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#define DVR_DEV "/dev/dvb/card0/dvr1"
#define DEMUX1_DEV "/dev/dvb/card0/demux1"
#else
#include <linux/dvb/dmx.h>
	#if HAVE_DBOX2_DRIVER
		#define DVR_DEV "/dev/dvb/adapter0/dvr0"
		#define DEMUX1_DEV "/dev/dvb/adapter0/demux0"
	#else
		#define DVR_DEV "/dev/dvb/adapter0/dvr1"
		#define DEMUX1_DEV "/dev/dvb/adapter0/demux1"
	#endif
#endif

extern ePermanentTimeshift permanentTimeshift;

static pthread_mutex_t PMTLock =
	PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

inline void lock_pmt()
{
	pthread_mutex_lock(&PMTLock);
}

inline void unlock_pmt(void*)
{
	pthread_mutex_unlock(&PMTLock);
}

static int section_length(const unsigned char *buf)
{
	return ((buf[1] << 8) | buf[2]) & 0x0fff;
}

static int ts_header(unsigned char *dest, int pusi, int pid, int scrmbl, int adap, unsigned int &cc)
{
	dest[0] = 0x47;
	dest[1] = (!!pusi << 6) | (pid >> 8);
	dest[2] = pid;
	dest[3] = (scrmbl << 6) | (adap << 4) | (cc++ & 0x0f);

	return 4;
}

static int section2ts(unsigned char *dest, const unsigned char *src, int pid, unsigned int &ccount )
{
	unsigned char *orig = dest;
	int pusi = 1;
	int len, cplen;

	orig = dest;

	for (len = section_length(src) + 3; len > 0; len -= cplen) {
		dest += ts_header(dest, pusi, pid, 0, 1, ccount);

		if (pusi) {
			*dest++ = 0x00;	/* pointer_field */
			cplen = MIN(len, 183);
			pusi = 0;
		}
		else {
			cplen = MIN(len, 184);
		}

		memcpy(dest, src, cplen);
		dest += cplen;
		src += cplen;
	}

	if ((cplen = (dest - orig) % 188)) {
		cplen = 188 - cplen;
		memset(dest, 0xff, cplen);
		dest += cplen;
	}

	return dest - orig;
}

int eDVBRecorder::flushBuffer()
{
	if (!bufptr)
		return 0;

	int towrite = splitsize-size>bufptr ? bufptr : splitsize-size;

	int retrycount=5; // 5 write retrys..
	int written=0;
	int dosplit = 0;
	while( written < towrite )
	{
		int wr = ::write(outfd, buf+written, towrite-written);
		if ( wr < towrite )  // to less bytes written?
		{
			if ( wr < 0 )
			{
				if (errno == EINTR) continue;
				goto Error;
			}
			if ( !retrycount-- )
				goto Error;
		}
		written += wr;
		size += wr;
		written_since_last_sync += wr;
	}
	permanentTimeshift.lock.lock();
	if (permanentTimeshift.IsTimeshifting)
	{
		int minutes = 30;
		eConfig::getInstance()->getKey("/enigma/timeshift/permanentminutes", minutes );
		if (permanentTimeshift.CheckSlice(minutes))
		{
			if ( openFile(permanentTimeshift.getCurrentRecordingSlice()) ) 
				goto Error;
			dosplit = 1;
		}
		permanentTimeshift.lock.unlock();
	}
	else
	{
		permanentTimeshift.lock.unlock();
		if (size >= splitsize)
		{
			splitlock.lock();
			if ( openFile(++splits) ) // file creation failed?
				goto Error;
			splitlock.unlock();
			dosplit = 1;
		}
	}
	if (dosplit)
	{
		retrycount=5; // 5 write retrys..
		while ( written < bufptr )  // must flush remaining bytes from buffer..
		{
			towrite=bufptr-written;
			int wr = ::write(outfd, buf+written, towrite);
			if ( wr < towrite ) // to less bytes written?
			{
				if ( wr < 0 )
				{
					if (errno == EINTR) continue;
					goto Error;
				}
				if ( !retrycount-- )
					goto Error;
			}
			written += wr;
			written_since_last_sync += wr;
			size += wr;
		}
	}
	else
	{
		if (written_since_last_sync >= 1024*1024) // 1M
		{
			int toflush = written_since_last_sync > 1024*1024 ? 1024*1024 :
				written_since_last_sync & ~4095;
			off64_t dest_pos = lseek64(outfd, 0, SEEK_CUR);
			dest_pos -= toflush;
			posix_fadvise64(outfd, dest_pos, toflush, POSIX_FADV_DONTNEED);
			written_since_last_sync -= toflush;
		}
	}
	bufptr=0;
	return 0;

Error:
	splitlock.unlock();
	permanentTimeshift.lock.unlock();
	eDebug("recording write error, maybe disk full");
	state = stateError;
	rmessagepump.send(eDVBRecorderMessage(eDVBRecorderMessage::rWriteError));
	return -1;
}

void SAHandler(int ptr)
{
	if ( eDVB::getInstance() && eDVB::getInstance()->recorder )
		eDVB::getInstance()->recorder->setWritePatPmtFlag();
}


#ifndef EBUFFEROVERFLOW
#define EBUFFEROVERFLOW  769
#endif

void eDVBRecorder::thread()
{
	signal(SIGALRM, SAHandler);
	PatPmtWrite();
	while (state == stateRunning)
	{
		int rd = 524144-bufptr;
		if ( rd > 65424 )
			rd = 65424;
		int r = ::read(dvrfd, buf+bufptr, rd);
		if ( r < 0 )
		{
#if HAVE_DVB_API_VERSION < 3
			// workaround for driver bug....
			if (r == -EBUFFEROVERFLOW)
				continue;
#endif
			switch(errno)
			{
				case EINTR:
				case EOVERFLOW:
					continue;
				default:
					/*
					 * any other error will immediately cause the same error
					 * when we would call 'read' again with the same arguments
					 */
					eDebug("recording read error %d", errno);
					state = stateError;
					rmessagepump.send(eDVBRecorderMessage(eDVBRecorderMessage::rWriteError));
					break;
			}
			break;
		}
		/* note that some dvr devices occasionally return EOF, we should ignore that */

		bufptr += r;

		if ( writePatPmt )  // is set in SAHandler
		{
			PatPmtWrite();
			writePatPmt=false;
		}
		else if ( bufptr > 524143 )
			flushBuffer();
	}
	alarm(0);
	signal(SIGALRM, SIG_DFL );
}

void eDVBRecorder::PMTready(int error)
{
	eDebug("eDVBRecorder PMTready");
	if ( !error )
	{
		PMT *pmt=tPMT.ready()?tPMT.getCurrent():0;
		if ( pmt )
		{
			eDVBCaPMTClientHandler::distribute_gotPMT(recRef, pmt);
			eDebug("UpdatePIDs");
//			addNewPID(0); // PAT
//			addNewPID(pmt->pid);  // PMT
			addNewPID(pmt->PCR_PID);  // PCR

			for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end(); ++i)
			{
				int record=0;
				int isUndocumentedNA = 0;
				switch (i->stream_type)
				{
					case 1:	// video..
					case 2:
						record=1;
						break;
					case 3:	// audio..
					case 4:
						record=1;
						break;
					case 0x80:
					case 0x81:
					case 0x82:
					case 0x83:
						isUndocumentedNA = 1;
					case 6:
					for (ePtrList<Descriptor>::iterator it(i->ES_info); it != i->ES_info.end(); ++it)
					{
						switch (it->Tag())
						{
							case DESCR_AC3:
							{
								int norecord=0;
								eConfig::getInstance()->getKey("/enigma/noac3recording", norecord);
								if (!norecord)
									record=1;
								break;
							}
							case DESCR_ISO639_LANGUAGE:
								if (isUndocumentedNA)
								{
									record = 1;
								}
								break;
#ifdef RECORD_TELETEXT
							case DESCR_TELETEXT:
							{
								int norecord=0;
								eConfig::getInstance()->getKey("/enigma/nottxrecording", norecord);
								if (!norecord)
									record=2;  // low bti
								break;
							}
#endif
#ifdef RECORD_SUBTITLES
							case DESCR_SUBTITLING:
							{
								record=2;
								break;
							}
#endif
						}
					}
					break;
				}
				if (record)
					addNewPID(i->elementary_PID, record==2?DMX_LOW_BITRATE:0);
#ifdef RECORD_ECM
				for (ePtrList<Descriptor>::iterator it(i->ES_info); it != i->ES_info.end(); ++it)
					if (it->Tag() == 9)
						addNewPID(((CADescriptor*)*it)->CA_PID);
#endif
			}
			validatePIDs();

			pthread_cleanup_push( unlock_pmt, 0 );
			lock_pmt();
			if (PmtData) delete [] PmtData;
			PmtData = pmt->getRAW();
			pthread_cleanup_pop(1);

			pmt->unlock();
		}
	}
}

void eDVBRecorder::gotBackMessage(const eDVBRecorderMessage &msg)
{
	switch (msg.code)
	{
	case eDVBRecorderMessage::rWriteError:
		/* emit */ recMessage(recWriteError);
		break;
	default:
		break;
	}
}

int eDVBRecorder::openFile(int suffix)
{
	eString tfilename=filename;
	if (suffix)
		tfilename+=eString().sprintf(".%03d", suffix);

	size=0;
	written_since_last_sync=0;

	if (outfd >= 0)
		::close(outfd);

	struct stat64 s;
	if ( !stat64(tfilename.c_str(), &s) )
	{
		rename(tfilename.c_str(), (tfilename+".$$$").c_str() );
		if (eBackgroundFileEraser::getInstance())
			eBackgroundFileEraser::getInstance()->erase((tfilename+".$$$").c_str());
	}

#ifdef HAVE_DREAMBOX_HARDWARE
	outfd=::open(tfilename.c_str(), O_CREAT|O_WRONLY|O_TRUNC|O_LARGEFILE, 0664);
#else
	if (access("/var/etc/.no_o_sync", R_OK) == 0)
		outfd=::open(tfilename.c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_LARGEFILE, 0664);
	else
		outfd=::open(tfilename.c_str(),O_SYNC|O_CREAT|O_WRONLY|O_TRUNC|O_LARGEFILE, 0664);
#endif

	if (outfd < 0)
	{
		eDebug("failed to open DVR file: %s (%m)", tfilename.c_str());	
		return -1;
	}

	/* turn off kernel caching strategies */
	posix_fadvise64(outfd, 0, 0, POSIX_FADV_RANDOM);

	return 0;
}

void eDVBRecorder::open(const char *_filename)
{
	eDebug("eDVBRecorder::open(%s)", _filename);
	pids.clear();
	newpids.clear();

	filename=_filename;

	int tmp=1024*1024;  // 1G
	if (eConfig::getInstance()) eConfig::getInstance()->getKey("/extras/record_splitsize", tmp);

	splitsize=tmp;
	splitsize*=1024;
	splitsize/=188;  // align to 188 bytes..
	splitsize*=188;

	openFile(splits=0);

	if ( dvrfd >= 0 )
		::close(dvrfd);

	dvrfd=::open(DVR_DEV, O_RDONLY);
	if (dvrfd < 0)
	{
		eDebug("failed to open "DVR_DEV" (%m)");
		::close(outfd);
		outfd=-1;
		return;
	}
	if (outfd < 0)
		rmessagepump.send(eDVBRecorderMessage(eDVBRecorderMessage::rWriteError));
}

std::pair<std::set<eDVBRecorder::pid_t>::iterator,bool> eDVBRecorder::addPID(int pid, int flags)
{
	eDebugNoNewLine("eDVBRecorder::addPID(0x%x)...", pid );
	pid_t p;
	p.pid=pid;
	if ( pids.find(p) != pids.end() )
	{
		eDebug("we already have this pid... skip!");
		return std::pair<std::set<pid_t>::iterator, bool>(pids.end(),false);
	}
	p.fd=::open(DEMUX1_DEV, O_RDWR);
	if (p.fd < 0)
	{
		eDebug("failed to open demux1");
		return std::pair<std::set<pid_t>::iterator, bool>(pids.end(),false);
	}
#if HAVE_DVB_API_VERSION < 3
	dmxPesFilterParams flt;
	flt.pesType=DMX_PES_OTHER;
#else
	dmx_pes_filter_params flt;
	flt.pes_type=DMX_PES_OTHER;
#endif
	flt.pid=p.pid;
	flt.input=DMX_IN_FRONTEND;
	flt.output=DMX_OUT_TS_TAP;

	flt.flags=flags;
	eDebug("flags %08x", flags);

	if (::ioctl(p.fd, DMX_SET_PES_FILTER, &flt)<0)
	{
		eDebug("DMX_SET_PES_FILTER failed (for pid %d)", flt.pid);
		::close(p.fd);
		return std::pair<std::set<pid_t>::iterator, bool>(pids.end(),false);
	}

	return pids.insert(p);
}

void eDVBRecorder::addNewPID(int pid, int flags)
{
	pid_t p;
	p.pid = pid;
	p.flags = flags;
	newpids.insert(p);
}

void eDVBRecorder::validatePIDs()
{
	for (std::set<pid_t>::iterator it(pids.begin()); it != pids.end();)
	{
		std::set<pid_t>::iterator i = newpids.find(*it);
		if ( i == newpids.end() )  // no more existing pid...
			removePID((it++)->pid);
		else
			++it;
	}
	for (std::set<pid_t>::iterator it(newpids.begin()); it != newpids.end(); ++it )
	{
		std::pair<std::set<pid_t>::iterator,bool> newpid = addPID(it->pid, it->flags);
		if ( newpid.second )
		{
			if ( state == stateRunning )
			{
				if (newpid.first->fd >= 0)
				{
					if (::ioctl(newpid.first->fd, DMX_START, 0)<0)
					{
						eDebug("DMX_START failed (%m)");
						removePID(it->pid);
					}
				}
			}
		}
		else
			eDebug("error during add new pid");
	}
	newpids.clear();
}

void eDVBRecorder::removePID(int pid)
{
	pid_t p;
	p.pid=pid;
	std::set<pid_t>::iterator pi=pids.find(p);
	if (pi != pids.end())
	{
		if (pi->fd >= 0)
			::close(pi->fd);
		pids.erase(pi);
		eDebug("eDVBRecorder::removePID(0x%x)", pid);
	}
}

void eDVBRecorder::start()
{
	if ( state == stateRunning )
		return;

	eDebug("eDVBRecorder::start()");

	state = stateRunning;

	if ( !thread_running() )
	{
		eDebug("run thread");
		run();
	}

	for (std::set<pid_t>::iterator i(pids.begin()); i != pids.end(); ++i)
	{
		eDebug("starting pidfilter for pid %d", i->pid );

		if (i->fd >= 0)
		{
			if (::ioctl(i->fd, DMX_START, 0)<0)
			{
				eDebug("DMX_START failed");
				::close(i->fd);
				pids.erase(i);
			}
		}
	}
}

void eDVBRecorder::stop()
{
	if ( state == stateStopped )
		return;

	eDebug("eDVBRecorder::stop()");

	state = stateStopped;

	int timeout=20;
	while ( thread_running() && timeout )
	{
		usleep(100000);  // 2 sec time for thread shutdown
		--timeout;
	}

	for (std::set<pid_t>::iterator i(pids.begin()); i != pids.end(); ++i)
		if (i->fd >= 0)
			::ioctl(i->fd, DMX_STOP, 0);

	flushBuffer();
}

void eDVBRecorder::close()
{
	if (state != stateStopped)
		stop();

	eDebug("eDVBRecorder::close()");

	for (std::set<pid_t>::iterator i(pids.begin()); i != pids.end(); ++i)
		if (i->fd >= 0)
			::close(i->fd);

	pids.clear();

	if (dvrfd >= 0)
	{
		::close(dvrfd);
		dvrfd = -1;
	}
	if (outfd >= 0)
	{
		::close(outfd);
		outfd = -1;
	}

	if ( thread_running() )
		kill(true);
}

eDVBRecorder::eDVBRecorder(PMT *pmt,PAT *pat)
:state(stateStopped), rmessagepump(eApp, 1), dvrfd(-1) ,outfd(-1)
,bufptr(0), PmtData(NULL), PatData(NULL)
,PmtCC(0), PatCC(0), writePatPmt(true)
{
	init_eDVBRecorder(pmt,pat);
}
void eDVBRecorder::init_eDVBRecorder(PMT *pmt,PAT *pat)
{
	CONNECT(rmessagepump.recv_msg, eDVBRecorder::gotBackMessage);
	rmessagepump.start();

	if (pmt)
	{
		CONNECT( tPMT.tableReady, eDVBRecorder::PMTready );
		tPMT.start((PMT*)pmt->createNext(), DEMUX1_DEV );
		PmtData=pmt->getRAW();
		pmtpid=pmt->pid;
		if (pat)
		{
			PAT p;
			p.entries.setAutoDelete(false);
			p.version=pat->version;
			p.transport_stream_id=pat->transport_stream_id;
			for (ePtrList<PATEntry>::iterator it(pat->entries);
				it != pat->entries.end(); ++it)
			{
				if ( it->program_number == pmt->program_number )
				{
					p.entries.push_back(*it);
					PatData=p.getRAW();
					break;
				}
			}
		}
	}
}

eDVBRecorder::~eDVBRecorder()
{
	if (PatData) delete [] PatData;
	if (PmtData) delete [] PmtData;
	eDVBServiceController *sapi = NULL;
	if (eDVB::getInstance()) sapi = eDVB::getInstance()->getServiceAPI();

	if (sapi)
	{
		// workaround for faked service types..
		eServiceReferenceDVB tmp = sapi->service;
		tmp.data[0] = recRef.getServiceType();
	
		if ( tmp != recRef )
		{
			eServiceReferenceDVB ref=sapi->service;
			eDVBCaPMTClientHandler::distribute_leaveService(recRef);
		}
	}
	close();
}

void eDVBRecorder::writeSection(void *data, int pid, unsigned int &cc)
{
	if ( !data )
		return;

	__u8 secbuf[4300];  // with ts overhead...

	int len = section2ts(secbuf, (__u8*)data, pid, cc);

	if ( len )
	{
		if ( (bufptr+len) > 524143 )
			flushBuffer();

		memcpy(buf+bufptr, secbuf, len);
		bufptr+=len;
	}
}

void eDVBRecorder::PatPmtWrite()
{
//eDebug("PatPmtWrite");
	if ( PatData )
		writeSection(PatData, 0, PatCC );

	pthread_cleanup_push( unlock_pmt, 0 );
	lock_pmt();
	if ( PmtData )
		writeSection(PmtData, pmtpid, PmtCC );
	pthread_cleanup_pop(1);

	alarm(2);  
}
void eDVBRecorder::SetSlice(int slice)
{
	splitlock.lock();
	eString oldfilename=filename;
	if (splits)
		oldfilename+=eString().sprintf(".%03d", splits);
	splits = slice;
	splitlock.unlock();
	eString newfilename=filename;
	if (slice)
		newfilename+=eString().sprintf(".%03d", slice);
eDebug("rename current recording:%s|%s",oldfilename.c_str(),newfilename.c_str());
	::rename(oldfilename.c_str(),newfilename.c_str());
}

#endif //DISABLE_FILE
