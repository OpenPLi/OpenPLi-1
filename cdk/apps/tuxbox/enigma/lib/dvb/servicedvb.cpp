#define TIMESHIFT
		// intial time to lag behind live
#define TIMESHIFT_GUARD (1024*1024)
#include <lib/base/i18n.h>
#include <lib/dvb/servicedvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/frontend.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/info.h>
#include <lib/system/dmfp.h>
#include <lib/system/elock.h>
#include <lib/driver/streamwd.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/servicefile.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/decoder.h>
#include <src/media_mapping.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>

#if HAVE_DVB_API_VERSION < 3
#include <ost/audio.h>
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#else
#include <linux/dvb/audio.h>
#define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#endif

#ifndef DISABLE_FILE
#include <lib/dvb/record.h>
#include <lib/system/file_eraser.h>

void ePermanentTimeshift::Start()
{
	lock.lock();
	eDebug("[PERM] starting permanent timeshift:%d",IsTimeshifting);
	slicelist.clear();
	slicelist.push_back(std::pair<int,off64_t>(0,0));
	current_slice_playing = slicelist.end();
	IsTimeshifting = 1;
	gettimeofday(&(last_split),0);

	struct stat64 s;
	eString timeshiftDir;
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaTimeshifts, timeshiftDir);
	
	if(stat64(timeshiftDir.c_str(), &s))
	{
		// No timeshift directory present, create it now
		system(("mkdir " + timeshiftDir).c_str());
	}

	int minutes = 30;
	eConfig::getInstance()->getKey("/enigma/timeshift/permanentminutes", minutes );
	eString path = getTimeshiftPath();	
	eString filename = eString().sprintf("%s.%03d", path.c_str(), minutes);
	while (!stat64(filename.c_str(), &s))
	{
		eBackgroundFileEraser::getInstance()->erase(filename.c_str());
		minutes++;
		filename = eString().sprintf("%s.%03d", path.c_str(), minutes);
	}

	lock.unlock();
}
void ePermanentTimeshift::Stop()
{
	lock.lock();
	eDebug("[PERM] stopping permanent timeshift:%d",IsTimeshifting);
	IsTimeshifting = 0;
	eString path = getTimeshiftPath();
	if (slicelist.size() > 0)
	{
		int slice = slicelist.back().first;
		struct stat64 s;
		eString filename = (slice ? eString().sprintf("%s.%03d", path.c_str(), slice) : path);
		if (!stat64(filename.c_str(), &s))
		{
			slicelist.back().second=s.st_size;
			eDebug("[PERM] remember slice:%d,%d",slicelist.back().first,s.st_size);
		}
	}
	lock.unlock();

}
bool ePermanentTimeshift::CheckSlice(unsigned int minutes)
{
	timeval now;
	gettimeofday(&now,0);
	// new File every 60 seconds of recording
	if ((now.tv_sec - last_split.tv_sec) >= 60)
	{
		eString path = getTimeshiftPath();
		int slice = slicelist.back().first;
		struct stat64 s;
		eString filename = (slice ? eString().sprintf("%s.%03d", path.c_str(), slice) : path);
		if (!stat64(filename.c_str(), &s))
		{
			slicelist.back().second=s.st_size;
			eDebug("[PERM] remember slice:%d,%d",slicelist.back().first,s.st_size);
		}
		int nextslice = slicelist.size();
		if (slicelist.size() >= minutes && current_slice_playing != slicelist.begin())
		{
			nextslice = slicelist.front().first;			
			slicelist.pop_front();
			//if (current_slice_playing != slicelist.end())
			//	current_slice_playing--;
		}
		slicelist.push_back(std::pair<int,off64_t>(nextslice,0));
		last_split = now;
		return true;
	}
	return false;
}
int ePermanentTimeshift::getRecordedMinutes()
{
	return slicelist.size();
}
void ePermanentTimeshift::renameAllSlices(const char* filename)
{
	eDebug("renameAllSlices:%s",filename);
	int slice = 0;
	eString path = getTimeshiftPath();
	for (std::list<std::pair<int,off64_t> >::iterator x(slicelist.begin()); x != slicelist.end(); ++x)
	{
		eString oldfilename = (x->first ? eString().sprintf("%s.%03d", path.c_str(), x->first) : path.c_str());
		eString newfilename=filename;
		if (slice)
			newfilename += eString().sprintf(".%03d", slice);
		eString cmd;
		cmd.sprintf("mv \"%s\" \"%s\"",oldfilename.c_str(),newfilename.c_str());		
		if (system(cmd.c_str()))
		{
			eDebug("renaming timeshift failed:%s:%s",cmd.c_str(),strerror(errno));
		}

		slice++;
	}
	slicelist.clear();
}
off64_t ePermanentTimeshift::getCurrentLength(int slice)
{
	lock.lock();
	off64_t filelength = 0;
	for (std::list<std::pair<int,off64_t> >::iterator x(slicelist.begin()); x != slicelist.end(); ++x)
	{
		if (x->first == slice)
			break;
		filelength += x->second;
	}
	if (slice < 0)
	{
		eString path = getTimeshiftPath();
		struct stat64 s;
		eString curfilename = (slicelist.back().first ? eString().sprintf("%s.%03d", path.c_str(), slicelist.back().first) : path);
		if (!stat64(curfilename.c_str(), &s))
		{
			filelength+=s.st_size;
		}
	}
	lock.unlock();
	//eDebug("[PERM] receiving filelength:%d,%lld",slice,filelength);
	return filelength;
}
int ePermanentTimeshift::setNextPlayingSlice()
{
	lock.lock();
	int slice = -1;
	if (current_slice_playing != slicelist.end())
	{
		current_slice_playing++;
		if (current_slice_playing != slicelist.end())
			slice = current_slice_playing->first;

	}	
	lock.unlock();
	eDebug("[PERM] setting next slice:%d",slice);
	return slice;
}
int ePermanentTimeshift::getCurrentPlayingSlice()
{
	int slice = -1;
	if (current_slice_playing == slicelist.end())
		current_slice_playing =slicelist.begin();
	slice = current_slice_playing->first;
	eDebug("[PERM] getting current playing slice:%d",slice);
	return slice;
}
off64_t ePermanentTimeshift::seekTo(off64_t offset)
{
	off64_t newoffset = 0;
	off64_t currentoffset = 0;
	bool bSet = false;
	for (std::list<std::pair<int,off64_t> >::iterator x(slicelist.begin()); x != slicelist.end(); ++x)
	{
		currentoffset += x->second;
		if (currentoffset > offset)
		{
			if (current_slice_playing != x)
			{
				current_slice_playing = x;
			}
			newoffset = offset - (currentoffset - x->second) ;
			bSet = true;
			break;
		}
	}
	if (!bSet && slicelist.size() > 0)
	{
		current_slice_playing = slicelist.end();
		current_slice_playing--;
		newoffset = offset - currentoffset;
	}
	eDebug("[PERM] seekTo:%lld(%lld),%d",offset,newoffset,current_slice_playing->first);
	return newoffset;
}
eString ePermanentTimeshift::getTimeshiftPath()
{
	eString mountPoint;
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaTimeshifts, mountPoint);
	return mountPoint + "/timeshift";
}
ePermanentTimeshift permanentTimeshift;

eDVRPlayerThread::eDVRPlayerThread(const char *_filename, eServiceHandlerDVB *handler, int livemode, int playingPermanentTimeshift)
	:handler(handler), buffer(65424), livemode(livemode), playingPermanentTimeshift(playingPermanentTimeshift), liveupdatetimer(this), curBufferFullness(0)
	,needasyncworkaround(false), inputsn(0), outputsn(0), messages(this, 1)
{
	init_eDVRPlayerThread(_filename);
}
void eDVRPlayerThread::init_eDVRPlayerThread(const char *_filename)
{
	state=stateInit;
	int nodetect=0;
	eConfig::getInstance()->getKey("/enigma/notimestampdetect", nodetect );
	timestampParser = nodetect ? 0 : new eTimeStampParserTS(_filename);
		

	int count=0;
	seekbusy=0;
	seeking=0;
#if HAVE_DVB_API_VERSION < 3
	do
	{
		dvrfd=::open("/dev/pvr", O_WRONLY|O_NONBLOCK); // TODO: change to /dev/dvb/dvr0 (but only when drivers support this!)
		if (dvrfd < 0)
		{
			if ( errno == EBUSY )
			{
				eDebug("pvr device busy try %d", count++);
				if ( count < 40 )
				{
					usleep(20000);
					continue;
				}
			}
			eDebug("couldn't open /dev/pvr - buy the new $$$ box and load pvr.o! (%m)");
			state=stateError;
		}
		break;
	}
	while( dvrfd < 0 );
#else
	if ((dvrfd = ::open("/dev/dvb/adapter0/dvr0", O_WRONLY|O_NONBLOCK)) == -1) 
	{
		eDebug("couldn't open /dev/dvb/adapter0/dvr0 (%m)");
		state=stateError;
	}
#endif
	outputsn=new eSocketNotifier(this, dvrfd, eSocketNotifier::Write, 0);
	CONNECT(outputsn->activated, eDVRPlayerThread::outputReady);

	CONNECT(liveupdatetimer.timeout, eDVRPlayerThread::updatePosition);

	filename=_filename;

	sourcefd=-1;
	inputsn=0;

	slice=0;
	audiotracks=1;

	if (playingPermanentTimeshift)
	{
		filelength = permanentTimeshift.getCurrentLength (-1)/1880;
	}
	else
	{
		filelength = FillSliceSizes();
	}

	if (openFile(0))
	{
		state=stateError;
		eDebug("error opening %s (%m)", filename.c_str());
	}

	CONNECT(messages.recv_msg, eDVRPlayerThread::gotMessage);

	if ( eSystemInfo::getInstance()->getHwType() < 3 ) // dbox2
		maxBufferFullness=128*1024;
	else
		maxBufferFullness=256*1024;

	speed=1;

	run();

	if (livemode)
	{
		int fileend;
		if (filelength > (TIMESHIFT_GUARD/1880))
			fileend = filelength - (TIMESHIFT_GUARD/1880);
		else
			fileend = 0;
		if ( livemode == 1 )
			messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekreal, fileend));
	}

	FILE *bitstream=fopen("/proc/bus/bitstream", "rt");
	if (bitstream)
	{
		char buf[100];
		while (fgets(buf, 100, bitstream))
		{
			if (strstr(buf, "AUDIO_STC:"))
			{
				needasyncworkaround=1;
				break;
			}
		}
		fclose(bitstream);
	}
}
int eDVRPlayerThread::FillSliceSizes()
{
	slicesizes.clear();
	struct stat64 s;
	int filelength=0;
	int slice = 0;
	while (!stat64((filename + (slice ? eString().sprintf(".%03d", slice) : eString(""))).c_str(), &s))
	{
		filelength+=s.st_size/1880;
		slicesizes.push_back(s.st_size);
		slice++;
	}
	//eDebug("FillSliceSizes:%s,%d",filename.c_str(),filelength);
	return filelength;
}
int eDVRPlayerThread::openFile(int slice)
{
	eString tfilename=filename;
	if (slice)
		tfilename += eString().sprintf(".%03d", slice);

	if (inputsn)
	{
		delete inputsn;
		inputsn=0;
	}
	if (sourcefd >= 0)
		::close(sourcefd);

	sourcefd=::open(tfilename.c_str(), O_RDONLY|O_LARGEFILE);
	if (sourcefd >= 0)
	{
		off64_t slicesize=this->slicesize;
		eDebug("opened slice %d", slice);
		if (!slice && !playingPermanentTimeshift)
		{
			if (!livemode)
			{
				slicesize=lseek64(sourcefd, 0, SEEK_END);
				if (slicesize <= 0)
				{
					int tmp=1024*1024;
					eConfig::getInstance()->getKey("/extras/record_splitsize", tmp);
					slicesize=tmp;
					slicesize*=1024;
				}
				lseek64(sourcefd, 0, SEEK_SET);
			}
			else
			{
				int tmp=1024*1024;
				eConfig::getInstance()->getKey("/extras/record_splitsize", tmp);
				slicesize=tmp;
				slicesize*=1024;
			}
		}

		inputsn=new eSocketNotifier(this, sourcefd, eSocketNotifier::Read, 0);
		inputsn->start();
		CONNECT(inputsn->activated, eDVRPlayerThread::readMore);

		this->slicesize = slicesize;
		this->slice = slice;
		position=0;
		return 0;
	}
	return -1;
}

void eDVRPlayerThread::thread()
{
	exec();
}

void eDVRPlayerThread::outputReady(int what)
{
	(void)what;
	char ptsbuf[65424];
	int len = buffer.peek(ptsbuf, 65424);
	if (timestampParser)
		timestampParser->parseData(ptsbuf,len);
	int wr=buffer.tofile(dvrfd,65424);
	seekbusy-=wr;
	int newBufferFullness = curBufferFullness - wr;
	curBufferFullness = newBufferFullness;

	if (seekbusy < 0)
		seekbusy=0;
	if ((state == stateBufferFull) && (curBufferFullness<maxBufferFullness))
	{
		state=statePlaying;
		if (inputsn)
			inputsn->start();
	}
	if (!curBufferFullness)
	{
		eDebug("buffer empty, state %d", state);
 		outputsn->stop();
		if (state != stateFileEnd)
		{
			if (inputsn)
				inputsn->start();
			state=stateBuffering;
		} else
		{
			if (seeking)
			{
				Decoder::stopTrickmode();
				seeking=0;
			}
			if (!livemode)
			{
				eDebug("ok, everything played..");
				handler->messages.send(eServiceHandlerDVB::eDVRPlayerThreadMessage(eServiceHandlerDVB::eDVRPlayerThreadMessage::done));
			} else
			{
				eDebug("liveeof");
				handler->messages.send(eServiceHandlerDVB::eDVRPlayerThreadMessage(eServiceHandlerDVB::eDVRPlayerThreadMessage::liveeof));
			}
		}
	}
}

void eDVRPlayerThread::dvrFlush()
{
#if HAVE_DVB_API_VERSION < 3
	if ( ::ioctl(dvrfd, 0)< 0 )
		eDebug("PVR_FLUSH_BUFFER failed (%m)");
#endif
	Decoder::flushBuffer();
}

void eDVRPlayerThread::readMore(int what)
{
	(void)what;
	if ((state != statePlaying) && (state != stateBuffering))
	{
		eDebug("wrong state (%d)", state);
		return;
	}

	int flushbuffer=0;

	if (curBufferFullness < maxBufferFullness)
	{
		int rd = buffer.fromfile(sourcefd, maxBufferFullness);
		int next=0;
		if ( rd < maxBufferFullness)
		{
			next=1;
			if (playingPermanentTimeshift)
			{
				int nextslice = permanentTimeshift.setNextPlayingSlice();
				if (nextslice == -1)
				{
					eDebug("no next file, stateFileEnd (previous state %d)", state);
					flushbuffer=1;
					if (inputsn)
						inputsn->stop();
					next = 0;
				}
				if (next && openFile(nextslice)) // if no next part found, else continue there...
					flushbuffer=1;
			}
			else
			{
				if (livemode)
				{
					struct stat64 s;
					eString tfilename=filename;
					tfilename += eString().sprintf(".%03d", slice+1);
	
					if (::stat64(tfilename.c_str(), &s) < 0)
					{
						eDebug("no next file, stateFileEnd (previous state %d)", state);
						flushbuffer=1;
						if (inputsn)
							inputsn->stop();
						next=0;
					}
				}
				if (next && openFile(slice+1)) // if no next part found, else continue there...
					flushbuffer=1;
			}
		}

		int newbuffsize = curBufferFullness + rd;
		curBufferFullness = newbuffsize;
		if (!next)
		{
			off64_t newpos = position + rd;
			position = newpos;
		}
	}

	int bla = eSystemInfo::getInstance()->getHwType() < 3 ? 100000 : 16384;

	if ( (state == stateBuffering && curBufferFullness > bla) || flushbuffer )
	{
		state=statePlaying;
		outputsn->start();
	}

	if (flushbuffer)
	{
		state=stateFileEnd;
		if (inputsn)
			inputsn->stop();
	}

	if ((state == statePlaying) && (curBufferFullness >= maxBufferFullness))
	{
		state=stateBufferFull;
		if (inputsn)
			inputsn->stop();
	}
}

eDVRPlayerThread::~eDVRPlayerThread()
{
//	messages.send(eDVRPlayerThreadMessage(eDVRPlayerThreadMessage::setSpeed, 0));
	messages.send(eDVRPlayerThreadMessage(eDVRPlayerThreadMessage::exit));
	kill(); 			// join the thread

	int fd = Decoder::getAudioDevice();
	bool wasOpen = fd != -1;
	if (!wasOpen)
		fd = open(AUDIO_DEV, O_RDWR);
	if (fd >= 0 && ioctl(fd, AUDIO_SET_MUTE, 0) < 0)
		eDebug("AUDIO_SET_MUTE error (%m)");
	if (!wasOpen && fd >= 0)
		close(fd);

	if (inputsn)
		delete inputsn;
	if (outputsn)
		delete outputsn;
	if (dvrfd >= 0)
		::close(dvrfd);
	if (sourcefd >= 0)
		::close(sourcefd);
	if (timestampParser)
		delete timestampParser;
	timestampParser = 0;
}

void eDVRPlayerThread::updatePosition()
{
	int filelength=0;

	if (timestampParser)
		timestampParser->RefreshEndTime();
	if (playingPermanentTimeshift)
		filelength = permanentTimeshift.getCurrentLength (-1)/1880;
	else
		filelength = FillSliceSizes();
	if (state == stateFileEnd)
	{
		eDebug("file end reached, retrying..");
		state = stateBuffering;
		if (inputsn)
			inputsn->start();
	}
	this->filelength=filelength;
}

int eDVRPlayerThread::getPosition(int real)
{
	int ret=0;
	if (!real && timestampParser)
	{
		ret = timestampParser->getSecondsCurrent();
		if (ret >= 0)
			return ret;
		ret = 0;
	}
	int bufferFullness=0;
	if ( needasyncworkaround )
	{
		if ( real )
			bufferFullness = getDriverBufferFullness();
	}
	else
		bufferFullness = getDriverBufferFullness();

	bufferFullness += curBufferFullness; // add enigma buffer fullness

	off64_t filelength = getCurrentSliceLength();
	ret = ((position-bufferFullness)/1880) + (filelength/1880);
	if (!real)
	{
		if (Decoder::current.vpid==-1) //Radiorecording
			ret /= 15;
		else
			ret /= 250;
	}
	return ret;
}
// returns sum of length of all files up to the current slice
off64_t eDVRPlayerThread::getCurrentSliceLength()
{
	off64_t filelength = 0;
	if (playingPermanentTimeshift)
	{
		filelength = permanentTimeshift.getCurrentLength (slice);
	}
	else
	{
		for (int i = 0; i < slice; i++)
		{
			filelength += slicesizes[i];
		}			
	}
	return filelength;
}

int eDVRPlayerThread::getLength(int real)
{
	if (real)
		return filelength;
	int ret = timestampParser ? timestampParser->getSecondsDuration() : -1;
	if (ret >= 0)
		return ret;
	if (Decoder::current.vpid==-1)
		return filelength/15; //Radiorecording
	else
		return filelength/250;
}

void eDVRPlayerThread::seekTo(off64_t offset)
{
	off64_t newoffset = 0;
	if (playingPermanentTimeshift)
	{
		permanentTimeshift.lock.lock();
		newoffset = permanentTimeshift.seekTo(offset);
		int newslice = permanentTimeshift.getCurrentPlayingSlice();
		permanentTimeshift.lock.unlock();

		if (newslice != slice)
		{
			if (openFile(newslice))
			{
				if (livemode)
				{
					// end of timeshifted recording reached
					state=stateFileEnd;
					return;
				}
				eDebug("open slice %d failed\n", newslice);
				state=stateError;
			}
		}
		eDebug("[PERM]newoffset:%lld,%d,%d",newoffset,slice, state == stateError);
	}
	else
	{
		off64_t sliceoffset = 0;
		int newslice = 0;
		while (newslice < (int)slicesizes.size())
		{
			if (offset > (sliceoffset + slicesizes[newslice]))
			{
				sliceoffset += slicesizes[newslice];
				newslice++;
			}
			else
				break;
		}			
		if (slice != newslice)
		{
			if (openFile(newslice))
			{
				if (livemode)
				{
					// end of timeshifted recording reached
					state=stateFileEnd;
					return;
				}
				eDebug("open slice %d failed\n", newslice);
				state=stateError;
			}
		}
		newoffset = offset-sliceoffset;
	}

	if (state != stateError)
	{
		off64_t newpos=::lseek64(sourcefd, newoffset, SEEK_SET);
		dvrFlush();
		position=newpos;
	}
}

int eDVRPlayerThread::getDriverBufferFullness()
{
	int bufferFullness=0, tmp=0;
	FILE *bitstream=fopen("/proc/bus/bitstream", "rt");
	if (bitstream)
	{
		char buf[100];
		while (fgets(buf, 100, bitstream))
		{
			if (sscanf(buf, "VIDEO_BUF_SIZE: %d", &tmp) == 1)
			{
				bufferFullness+=tmp;
//				eDebug("video buf size is %d", tmp);
			}
			else if (sscanf(buf, "AUDIO_BUF: %08x", &tmp) == 1)
			{
				bufferFullness+=tmp*audiotracks;
//				eDebug("audio buf size is %d", tmp);
			}
		}
		bufferFullness+=bufferFullness/8;   	// assume overhead..
		fclose(bitstream);
	}
	return bufferFullness;
}

void eDVRPlayerThread::gotMessage(const eDVRPlayerThreadMessage &message)
{
	switch (message.type)
	{
	case eDVRPlayerThreadMessage::updateAudioTracks:
		audiotracks = message.parm;
		break;
	case eDVRPlayerThreadMessage::start:
		if (!(inputsn && outputsn))
			break;
		if (state == stateInit)
		{
			state=stateBuffering;
			inputsn->start();
		}
		livemode=message.parm;
		if (livemode)
			liveupdatetimer.start(1000);
		else
			liveupdatetimer.stop();
		break;
	case eDVRPlayerThreadMessage::startPaused:
		if (!(inputsn && outputsn))
			break;
		pauseBufferFullness = getDriverBufferFullness();  // driver buffer
		pauseBufferFullness += curBufferFullness;  // add fullness of enigma buffer..
		inputsn->stop();
		outputsn->stop();
		state=statePause;
		Decoder::Pause();
		livemode=message.parm;
		if (livemode)
			liveupdatetimer.start(1000);
		else
			liveupdatetimer.stop();
		break;
	case eDVRPlayerThreadMessage::exit:
		quit();
		break;
	case eDVRPlayerThreadMessage::setSpeed:
		if (!(inputsn && outputsn))
			break;
		speed=message.parm;
		if (message.parm == 0)
		{
			if ((state==stateBuffering) || (state==stateBufferFull) || (state==statePlaying))
			{
				pauseBufferFullness = getDriverBufferFullness();  // driver buffer
				pauseBufferFullness += curBufferFullness;  // add fullness of enigma buffer..
				inputsn->stop();
				outputsn->stop();
				state=statePause;
				Decoder::Pause();
			}
		}
		else if (state == statePause || message.parm == 2)  // force play
		{
			off64_t offset=0;
//			eDebug("%d bytes in buffer", buffsize);
			offset += position + getCurrentSliceLength();
			offset-=pauseBufferFullness;	// calc buffersize ( of driver and enigma buffer )
			buffer.clear();  		// clear enigma dvr buffer
			dvrFlush();			// clear audio and video rate buffer
			if (!playingPermanentTimeshift)
				FillSliceSizes();
			seekTo(offset);
			inputsn->start();
			outputsn->start();
			speed=message.parm;
			state=stateBuffering;
			Decoder::Resume();
			curBufferFullness=0;
		}
		else
		{
			buffer.clear();
			dvrFlush();
			curBufferFullness=0;
		}
		break;
	case eDVRPlayerThreadMessage::seekmode:
		if (!(inputsn && outputsn))
			break;
		switch (message.parm)
		{
		case 0:
			if (seeking)
				Decoder::stopTrickmode();
			seeking=0;
			break;
		case 1:
			if (!seeking)
				Decoder::startTrickmode();
			seeking=1;
			break;
		}
		break;
	case eDVRPlayerThreadMessage::seek:
	case eDVRPlayerThreadMessage::skip:
	case eDVRPlayerThreadMessage::seekreal:
	{
		if (!(inputsn && outputsn))
			break;
		if (seekbusy)
			break;
		off64_t offset=0;
		if (message.type != eDVRPlayerThreadMessage::seekreal)
		{
			offset = (timestampParser ? timestampParser->getAverageBitrate() : -1);
			if (offset <= 0)
			{
				if (Decoder::current.vpid==-1) // Radio
					offset = 192*1024;// assuming 192kBit bitrate...
				else
					offset=3*1024*1024; // assuming 3MBit bitrate...
			}
			offset/=8000;
			offset*=message.parm -(message.parm >= 0 ? 0 : offset*12);
			buffer.clear();
			offset-=1000*1000; // account for pvr buffer
			if (message.type == eDVRPlayerThreadMessage::skip)
			{
				offset += position + getCurrentSliceLength();
			}
			if (offset<0)
				offset=0;
			curBufferFullness=0;
			seekbusy=256*1024; // next seek only after 128k (video) data
		} else
		{
			buffer.clear();
			Decoder::flushBuffer();
			offset=((off64_t)message.parm)*1880;
			curBufferFullness=0;
		}

		seekTo(offset);

		if (state == statePlaying)
		{
			if (inputsn)
				inputsn->start();
			state=stateBuffering;
		}
		break;
	}
	case eDVRPlayerThreadMessage::addPermanentTimeshiftToRecording:
		if (playingPermanentTimeshift)
			playingPermanentTimeshift = 0;
		else
			slice += message.parm;
		filename=handler->current_filename;
		filelength = FillSliceSizes();
		break;
	}
}

void eServiceHandlerDVB::gotMessage(const eDVRPlayerThreadMessage &message)
{
	if (message.type == eDVRPlayerThreadMessage::done)
	{
		state=stateStopped;
		serviceEvent(eServiceEvent(eServiceEvent::evtEnd));
	}
	else if (message.type == eDVRPlayerThreadMessage::liveeof)
		stopPlayback(1);
}

void eServiceHandlerDVB::handleDVBEvent( const eDVBEvent & e )
{
	switch ( e.type )
	{
		case eDVBEvent::eventRecordWriteError:
			serviceEvent(eServiceEvent(eServiceEvent::evtRecordFailed));
		break;
	}
}

void eServiceHandlerDVB::startPlayback(const eString &filename, int livemode, bool startpaused)
{
	stopPlayback();
	decoder=new eDVRPlayerThread(filename.c_str(), this, livemode,playingPermanentTimeshift);
	if (startpaused)
	{
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::startPaused, livemode));
	}
	else
	{
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::start, livemode));
	}
	flags=flagIsSeekable|flagSupportPosition;
	state= statePlaying;
	if ( livemode )
		flags|=flagStartTimeshift;
	pcrpid = Decoder::current.pcrpid;
		// stop pcrpid
	Decoder::parms.pcrpid = -1;
	Decoder::Set();
	serviceEvent(eServiceEvent(eServiceEvent::evtFlagsChanged) );
	if ( livemode )
		flags&=~flagStartTimeshift;
}

void eServiceHandlerDVB::stopPlayback( int waslivemode )
{
	if (decoder)
	{
			// reenable pcrpid
		Decoder::parms.pcrpid = pcrpid;
		Decoder::Set();
		flags&=~(flagIsSeekable|flagSupportPosition);
		serviceEvent(eServiceEvent(eServiceEvent::evtFlagsChanged) );
		delete decoder;
		decoder=0;
	}
}

#endif //DISABLE_FILE

int eServiceHandlerDVB::getID() const
{
	return eServiceReference::idDVB;
}

void eServiceHandlerDVB::scrambledStatusChanged(bool scrambled)
{
	int oldflags=flags;

	if (scrambled)
		flags |= flagIsScrambled;
	else
		flags &= ~flagIsScrambled;

	if (oldflags != flags)
		serviceEvent(eServiceEvent(eServiceEvent::evtFlagsChanged) );
}

void eServiceHandlerDVB::switchedService(const eServiceReferenceDVB &s, int err)
{
	if ( !s )
		return;
	int oldstate=state;
	error = err;
	if (error)
		state=stateError;
	else
	{
		if (state != statePause)
			state=statePlaying;
	}

	if (state != oldstate)
		serviceEvent(eServiceEvent(eServiceEvent::evtStateChanged));

	serviceEvent(eServiceEvent(eServiceEvent::evtStart));
}

void eServiceHandlerDVB::gotEIT(EIT *, int)
{
	serviceEvent(eServiceEvent(eServiceEvent::evtGotEIT));
}

void eServiceHandlerDVB::gotSDT(SDT *)
{
	serviceEvent(eServiceEvent(eServiceEvent::evtGotSDT));
}

void eServiceHandlerDVB::gotPMT(PMT *)
{
	serviceEvent(eServiceEvent(eServiceEvent::evtGotPMT));
	if ( decoder )
	{
		eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
		if ( sapi )
			decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::updateAudioTracks, sapi->audioStreams.size()));
	}
}

void eServiceHandlerDVB::leaveService(const eServiceReferenceDVB &e)
{
	if ( e )
		serviceEvent(eServiceEvent(eServiceEvent::evtStop));
}

void eServiceHandlerDVB::aspectRatioChanged(int isanamorph)
{
	aspect=isanamorph;
	serviceEvent(eServiceEvent(eServiceEvent::evtAspectChanged));
}

eServiceHandlerDVB::eServiceHandlerDVB()
	:eServiceHandler(eServiceReference::idDVB),
#ifndef DISABLE_FILE
	recording(0),
	messages(eApp, 0),
	decoder(0),
	playingPermanentTimeshift(0),
#endif
	cache(*this)
{
	//eDebug("[eServiceHandlerDVB] registering serviceInterface %d", id);
	init_eServiceHandlerDVB();
}
void eServiceHandlerDVB::init_eServiceHandlerDVB()
{
	if (eServiceInterface::getInstance()->registerHandler(id, this)<0)
		eFatal("couldn't register serviceHandler %d", id);

	CONNECT(eDVB::getInstance()->scrambled, eServiceHandlerDVB::scrambledStatusChanged);
	CONNECT(eDVB::getInstance()->switchedService, eServiceHandlerDVB::switchedService);
	CONNECT(eDVB::getInstance()->gotEIT, eServiceHandlerDVB::gotEIT);
	CONNECT(eDVB::getInstance()->gotSDT, eServiceHandlerDVB::gotSDT);
	CONNECT(eDVB::getInstance()->gotPMT, eServiceHandlerDVB::gotPMT);
	CONNECT(eDVB::getInstance()->leaveService, eServiceHandlerDVB::leaveService);
#ifndef DISABLE_FILE
	CONNECT(eDVB::getInstance()->eventOccured, eServiceHandlerDVB::handleDVBEvent);
#endif
	CONNECT(eStreamWatchdog::getInstance()->AspectRatioChanged, eServiceHandlerDVB::aspectRatioChanged);

	int data = 0xFFFFFFFF;
	data &= ~(1<<4);  // exclude NVOD
	data &= ~(1<<1);  // exclude TV
	data &= ~(1<<2);  // exclude Radio

	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF),
			new eService( _("Providers (TV)"))
		);
	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF ),
			new eService( _("Providers (Radio)"))
		);
	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, data, 0xFFFFFFFF),
			new eService( _("Providers (Data)"))
		);

	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, (1<<4)|(1<<1), 0xFFFFFFFF ), // TV and NVOD
			new eService( _("All services (TV)"))
		);
	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, 1<<2, 0xFFFFFFFF ), // radio
			new eService( _("All services (Radio)"))
		);
	cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, data, 0xFFFFFFFF),
			new eService( _("All services (Data)"))
		);

	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
	{
		cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, (1<<4)|(1<<1)),
			new eService( _("Satellites (TV)"))
		);
		cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, 1<<2),
			new eService( _("Satellites (Radio)"))
		);
		cache.addPersistentService(
			eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, data),
			new eService( _("Satellites (Data)"))
		);
	}
#ifndef DISABLE_FILE
	CONNECT(eServiceFileHandler::getInstance()->fileHandlers, eServiceHandlerDVB::addFile);
	recording=0;
	CONNECT(messages.recv_msg, eServiceHandlerDVB::gotMessage);
#endif
}

eServiceHandlerDVB::~eServiceHandlerDVB()
{
#ifndef DISABLE_FILE
	if (recording)
		eDVB::getInstance()->recEnd();
#endif
	if (eServiceInterface::getInstance()->unregisterHandler(id)<0)
		eFatal("couldn't unregister serviceHandler %d", id);
}

int eServiceHandlerDVB::play(const eServiceReference &service, int workaround )
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (service.type != eServiceReference::idDVB)
		return -1;
#ifndef DISABLE_FILE
	if ( !workaround )
		decoder=0;

	if (service.path.length())
	{
		if ( !workaround )
		{
			struct stat64 s;
			if (::stat64(service.path.c_str(), &s))
			{
				eDebug("file %s not exist.. don't play", service.path.c_str() );
				return -1;
			}
			if ( eDVB::getInstance()->recorder &&
				service.path == eDVB::getInstance()->recorder->getFilename() )
				startPlayback(service.path, 2);
			else
				startPlayback(service.path, 0);
		}
		else
			flags |= (flagIsSeekable|flagSupportPosition);
	}
	else
#endif
	{
		flags &= ~(flagIsSeekable|flagSupportPosition);
		serviceEvent(eServiceEvent(eServiceEvent::evtFlagsChanged) );
	}

	if (sapi)
		return sapi->switchService((const eServiceReferenceDVB&)service);

	return -1;
}

int eServiceHandlerDVB::serviceCommand(const eServiceCommand &cmd)
{
	switch (cmd.type)
	{
#ifndef DISABLE_FILE
	case eServiceCommand::cmdRecordOpen:
	{
		if (!recording)
		{
			permanentTimeshift.Stop();
			playingPermanentTimeshift = 0;
			char *filename=reinterpret_cast<char*>(cmd.parm);
			current_filename=filename;
			eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
			eDVB::getInstance()->recBegin(filename, sapi ? sapi->service : eServiceReferenceDVB());
			delete[] (filename);
			recording=1;
		} else
			return -1;
		break;
	}
	case eServiceCommand::cmdRecordOpenPermanentTimeshift:
	{
		if (!recording)
		{
			permanentTimeshift.Start();
			current_filename= permanentTimeshift.getTimeshiftPath();
			playingPermanentTimeshift = 1;
			eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
			eDVB::getInstance()->recBegin(current_filename.c_str(), sapi ? sapi->service : eServiceReferenceDVB());
			recording=1;
		} else
			return -1;
		break;
	}
	case eServiceCommand::cmdRecordStart:
		if (recording == 1)
		{
			eDVB::getInstance()->recResume();
			recording=2;
		} else	
			return -1;
		break;
	case eServiceCommand::cmdRecordStop:
		if (recording == 2)
		{
			eDVB::getInstance()->recPause();
			recording=1;
		} else
			return -1;
		break;
	case eServiceCommand::cmdRecordClose:
		if (recording)
		{
			permanentTimeshift.Stop();
			playingPermanentTimeshift = 0;
			recording=0;
			eDVB::getInstance()->recEnd();
		} else
			return -1;
		break;
	case eServiceCommand::cmdSetSpeed:
	{
		int parm=cmd.parm;
		if (!decoder && recording)
		{
			if ( parm == -1 ) // pause with must seek to begin
			{
				parm = 0;
				/* start paused playback */
				startPlayback(current_filename, 2, true);
			}
			else
				startPlayback(current_filename, 1);
		}
		if ((state == statePlaying) || (state == statePause) || (state == stateStopped) || (state == stateSkipping))
		{
			if (parm < 0 || !decoder)
				return -1;
			decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::setSpeed, parm));
			if (parm == 0)
				state=statePause;
			else if (parm == 1 || parm == 2)
				state=statePlaying;
			else
				state=stateSkipping;
		}
		else
			return -2;
		break;
	}
	case eServiceCommand::cmdSkip:
		if (!decoder)
		{
#ifdef TIMESHIFT
			if (recording && (cmd.parm < 0) )
				startPlayback(current_filename, 1);
			else
#endif
				return -1;
		}
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::skip, cmd.parm));
		break;
	case eServiceCommand::cmdSeekAbsolute:
		if (!decoder)
			return -1;
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekreal, cmd.parm));
		if ( cmd.parm == 0 && state == statePause )
			state = stateStopped;
		break;
	case eServiceCommand::cmdSeekReal:
		if (!decoder)
			return -1;
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekreal, cmd.parm));
		break;
	case eServiceCommand::cmdSeekBegin:
		if (!decoder)
			return -1;
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekmode, 1));
		break;
	case eServiceCommand::cmdSeekEnd:
		if (!decoder)
			return -1;
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekmode, 0));
		/* jumping to the current position after leaving seekmode avoids audio/video sync problems */
		decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::seekreal, decoder->getPosition(1)));
		break;
	case eServiceCommand::cmdAddPermanentTimeshiftToRecording:
		if (recording)
		{
			permanentTimeshift.lock.lock();
			int slices = permanentTimeshift.getRecordedMinutes();
			eDVB::getInstance()->recSetSlice(slices);
			permanentTimeshift.renameAllSlices(current_filename.c_str());
			if (decoder)
			{
				// currently playing timeshift file
				decoder->messages.send(eDVRPlayerThread::eDVRPlayerThreadMessage(eDVRPlayerThread::eDVRPlayerThreadMessage::addPermanentTimeshiftToRecording, slices));
			}
			permanentTimeshift.lock.unlock();
		}
		break;
#endif // DISABLE_FILE
	default:
		return -1;
	}
	return 0;
}

PMT *eServiceHandlerDVB::getPMT()
{
	return eDVB::getInstance()->getPMT();
}

void eServiceHandlerDVB::setPID(const PMTEntry *e)
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		sapi->setPID(e);
		sapi->setDecoder();
	}
}

SDT *eServiceHandlerDVB::getSDT()
{
	return eDVB::getInstance()->getSDT();
}

EIT *eServiceHandlerDVB::getEIT()
{
	return eDVB::getInstance()->getEIT();
}

int eServiceHandlerDVB::getAspectRatio()
{
	return aspect;
}

int eServiceHandlerDVB::getState()
{
	return state;
}

int eServiceHandlerDVB::getErrorInfo()
{
	return error;
}

int eServiceHandlerDVB::stop(int workaround)
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();

	if (sapi)
		sapi->switchService(eServiceReferenceDVB());

#ifndef DISABLE_FILE
	if (!workaround)
		stopPlayback();
#endif

	return 0;
}

#ifndef DISABLE_FILE
void eServiceHandlerDVB::addFile(void *node, const eString &filename)
{
	if (filename.right(3).upper()==".TS")
	{
//		struct stat64 s;
//		if (::stat64(filename.c_str(), &s))
//			return;
		eServiceFileHandler::getInstance()->addReference(node, eServiceReference(id, 0, filename));
	}
}
#endif

struct eServiceHandlerDVB_addService
{
	Signal1<void,const eServiceReference&> &callback;
	int type;
	int DVBNamespace;
	bool onlyNew;
	eServiceHandlerDVB_addService(Signal1<void,const eServiceReference&> &callback, int type, int DVBNamespace, bool onlyNew=false)
	: callback(callback), type(type), DVBNamespace(DVBNamespace), onlyNew(onlyNew)
	{
	}
	void operator()(const eServiceReference &service)
	{
		eService *s = eTransponderList::getInstance()->searchService( service );
		if ( !s )  // dont show "removed services"
			return;
		else if ( s->dvb && s->dvb->dxflags & eServiceDVB::dxDontshow )
			return;
		if ( onlyNew && !(s->dvb && s->dvb->dxflags & eServiceDVB::dxNewFound ) )
			return;
		int t = ((eServiceReferenceDVB&)service).getServiceType();
// hack for dish network TV service types!!!
		if ( type & (1<<1) ) // search for tv services ?
		{
			int onid = ((eServiceReferenceDVB&)service).getOriginalNetworkID().get();
			if (onid >= 0x1001 && onid <= 0x100b) // is dish network id?
			{
				static int dish_tv_types[] = { 128, 133, 137, 140, 144, 145, 150, 154, 160, 163, 164, 165, 166, 167, 168, 173, 174 };
				static size_t dish_tv_num_types = sizeof(dish_tv_types) / sizeof(int);
				if (std::binary_search(dish_tv_types, dish_tv_types + dish_tv_num_types, t))
					t = 1; // patch to tv service
			}
		}
///////////////////////////////////////////
		int nspace = ((eServiceReferenceDVB&)service).getDVBNamespace().get()&0xFFFF0000;
		if (t < 0)
			t=0;
		if (t >= 31)
			t=31;
		if ( type & (1<<t) && // right dvb service type
				 ( (DVBNamespace==(int)0xFFFFFFFF) || // ignore namespace
				 ( (DVBNamespace&(int)0xFFFF0000) == nspace ) // right satellite
				 )
			 )
			 callback(service);
	}
};

void eServiceHandlerDVB::enterDirectory(const eServiceReference &ref, Signal1<void,const eServiceReference&> &callback)
{
	switch (ref.type)
	{
	case eServiceReference::idDVB:
		switch (ref.data[0])
		{
		case -2:  // all TV or all Radio Services
			eTransponderList::getInstance()->forEachServiceReference(eServiceHandlerDVB_addService(callback, ref.data[1], ref.data[2] ));
			break;
		case -5:  // all TV or all Radio Services (only services marked as new)
			eTransponderList::getInstance()->forEachServiceReference(eServiceHandlerDVB_addService(callback, ref.data[1], ref.data[2], true ));
			break;
		case -3:  // normal dvb bouquet
		{
			eBouquet *b=eDVB::getInstance()->settings->getBouquet(ref.data[2]);
			if (!b)
				break;
			b->forEachServiceReference(eServiceHandlerDVB_addService(callback, ref.data[1], ref.data[3] ));
			break;
		}
		default:
			break;
		}
	default:
		break;
	}
	cache.enterDirectory(ref, callback);
}

eService *eServiceHandlerDVB::createService(const eServiceReference &node)
{
#ifndef DISABLE_FILE
	if (node.data[0]>=0)
	{
		eString l=node.path.mid(node.path.rfind('/')+1);
		if (!isUTF8(l))
			l=convertLatin1UTF8(l);
		if (node.descr)
			l=node.descr;
/* moved to dvbservice.cpp, function eDVBServiceController::FillPIDsFromFile
		int fd=open(node.path.c_str(), O_RDONLY|O_LARGEFILE);
		if (fd < 0)
			return 0;
		__u8 packet[188];
		eServiceDVB *dvb=0;
		do
		{
			if (::read(fd, packet, 188) != 188)
				break;
				// i know that THIS is not really a SIT parser :)
			if ((packet[0] != 0x47) || (packet[1] != 0x40) || (packet[2] != 0x1f) || (packet[3] != 0x10))
				break;
			int nameoffset = 6;
			if (memcmp(packet+0x15, "ENIGMA", 6))
			{
				//failed so check another
				if (!memcmp(packet+0x15, "NEUTRINONG", 10))
					nameoffset = 10;
				else
					break;
			}
			// we found our private descriptor:
			__u8 *descriptor=packet+0x13;
			int len=descriptor[1];
			dvb=new eServiceDVB(eServiceID((packet[0xf]<<8)|packet[0x10]), l.c_str());
			len-=nameoffset;
			descriptor+=2+nameoffset; // skip tag, len, ENIGMA or NEUTRINONG
			for (int i=0; i<len; i+=descriptor[i+1]+2)
			{
				int tag=descriptor[i];
				switch (tag)
				{
				case eServiceDVB::cVPID:
					dvb->set(eServiceDVB::cVPID, (descriptor[i+2]<<8)|(descriptor[i+3]));
					break;
				case eServiceDVB::cAPID:
					if (descriptor[i+4] == 0)
						dvb->set(eServiceDVB::cAPID, (descriptor[i+2]<<8)|(descriptor[i+3]));
					else
						dvb->set(eServiceDVB::cAC3PID, (descriptor[i+2]<<8)|(descriptor[i+3]));
					break;
				case eServiceDVB::cTPID:
					dvb->set(eServiceDVB::cTPID, (descriptor[i+2]<<8)|(descriptor[i+3]));
					break;
				case eServiceDVB::cPCRPID:
					dvb->set(eServiceDVB::cPCRPID, (descriptor[i+2]<<8)|(descriptor[i+3]));
					break;
				}
			}
		} while (0);
		close(fd);
		if (!dvb)
			return new eService(l);
		return dvb;
*/
		return new eServiceDVB(eServiceID(), l.c_str());
	}
#endif // DISABLE_FILE
	switch (node.data[0])
	{
	case -1: // for satellites...
	{
		std::map<int,tpPacket>::const_iterator it( eTransponderList::getInstance()->getNetworkNameMap().find( node.data[2] >> 16 ));
		if ( it == eTransponderList::getInstance()->getNetworkNameMap().end() )
			return 0;
		else
			return new eService( it->second.name+ _(" - provider"));
	}
	case -2: // for satellites...
	{
		std::map<int,tpPacket>::const_iterator it( eTransponderList::getInstance()->getNetworkNameMap().find( node.data[2] >> 16 ));
		if ( it == eTransponderList::getInstance()->getNetworkNameMap().end() )
			return 0;
		else
			return new eService( it->second.name+ _(" - services"));
	}
	case -5: // for satellites...
	{
		std::map<int,tpPacket>::const_iterator it( eTransponderList::getInstance()->getNetworkNameMap().find( node.data[2] >> 16 ));
		if ( it == eTransponderList::getInstance()->getNetworkNameMap().end() )
			return 0;
		else
			return new eService( it->second.name+ _(" - new found"));
	}
	case -6: 
	{
		return new eService( _("current transponder"));
	}
	case -3:
	{
		eBouquet *b=eDVB::getInstance()->settings->getBouquet(node.data[2]);
		if (!b)
			return 0;
		return new eService(b->bouquet_name.c_str());
	}
	}
	return 0;
}

struct eServiceHandlerDVB_SatExist
{
	std::set<int> &existingSats;
	eServiceHandlerDVB_SatExist(std::set<int> &existSats)
	: existingSats(existSats)
	{
	}
	void operator()(const eTransponder &tp)
	{
		if ( tp.satellite.isValid() )
			existingSats.insert(tp.satellite.orbital_position);
	}
};

void eServiceHandlerDVB::loadNode(eServiceCache<eServiceHandlerDVB>::eNode &node, const eServiceReference &ref)
{
	int data = 0xFFFFFFFF;
	data &= ~(1<<4);  // exclude NVOD
	data &= ~(1<<1);  // exclude TV
	data &= ~(1<<2);  // exclude Radio
	switch (ref.type)
	{
	case eServiceReference::idStructure:
		switch (ref.data[0])
		{
		case eServiceStructureHandler::modeBouquets:
			break;
		case eServiceStructureHandler::modeRoot:
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, (1<<4)|(1<<1) ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, 1<<2 ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, data ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, (1<<4)|(1<<1), 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, 1<<2, 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, data, 0xFFFFFFFF));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, data, 0xFFFFFFFF));
			break;
		case eServiceStructureHandler::modeTvRadio:
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, (1<<4)|(1<<1), 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, 1<<2, 0xFFFFFFFF ));
			break;
		case eServiceStructureHandler::modeTV:
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, (1<<4)|(1<<1) ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, (1<<4)|(1<<1), 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, (1<<4)|(1<<1), 0xFFFFFFFF ));
			break;
		case eServiceStructureHandler::modeRadio:
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, 1<<2 ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, 1<<2, 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, 1<<2, 0xFFFFFFFF ));
			break;
		case eServiceStructureHandler::modeData:
			if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -4, data ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -2, data, 0xFFFFFFFF ));
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, eServiceReference::flagDirectory|eServiceReference::shouldSort, -1, data, 0xFFFFFFFF ));
			break;
		}
		break;
	case eServiceReference::idDVB:
		switch (ref.data[0])
		{
		case -1:  // handle bouquets
		{
			std::map<int, eBouquet*> &map=*eDVB::getInstance()->settings->getBouquets();
			for (std::map<int, eBouquet*>::iterator x(map.begin()); x != map.end(); ++x)
			{
				eBouquet *i = x->second;
				int flags=eServiceReference::mustDescent|eServiceReference::canDescent|eServiceReference::isDirectory;

				if (i->bouquet_id >= 0) 		// sort only automatic generated services
					flags|=eServiceReference::shouldSort;

				for ( std::list<eServiceReferenceDVB>::iterator s(i->list.begin()); s != i->list.end(); ++s)
				{
					int t = s->getServiceType();
					int nspace = ((eServiceReferenceDVB&)*s).getDVBNamespace().get()&0xFFFF0000;
					if (t < 0)
						t=0;
					if (t >= 31)
						t=31;
					if ( ref.data[1] & (1<<t) && // right dvb service type
							 ( ( ref.data[2] == (int)0xFFFFFFFF) || // ignore namespace
								 ( (ref.data[2]&(int)0xFFFF0000) == nspace ) // right satellite
							 )
						 )
						 {
							 cache.addToNode(node, eServiceReference(eServiceReference::idDVB, flags, -3, ref.data[1], i->bouquet_id, ref.data[2] ));
							 break;
						 }
				}
			}
			break;
		}
		case -4:  // handle Satellites
		{
			static int flags=eServiceReference::mustDescent|eServiceReference::canDescent|eServiceReference::isDirectory|eServiceReference::shouldSort|eServiceReference::hasSortKey;
			std::set<int> filledSats;
			eTransponderList::getInstance()->forEachTransponder( eServiceHandlerDVB_SatExist( filledSats ));
			for ( std::set<int>::iterator it( filledSats.begin()) ; it != filledSats.end(); it++ )
			{
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, flags, -1, ref.data[1], *it<<16, *it ));
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, flags, -2, ref.data[1], *it<<16, *it ));
				cache.addToNode(node, eServiceReference(eServiceReference::idDVB, flags, -5, ref.data[1], *it<<16, *it ));
			}
			cache.addToNode(node, eServiceReference(eServiceReference::idDVB, flags, -6, ref.data[1], 0, 0 ));
			break;
		}
	}
	}
}

void eServiceHandlerDVB::leaveDirectory(const eServiceReference &dir)
{
	cache.leaveDirectory(dir);
}

eService *eServiceHandlerDVB::addRef(const eServiceReference &service)
{
	if ((service.data[0] < 0) || service.path)
	{
		eService *s=cache.addRef(service);
		return s;
	} else
	{
		eTransponderList *tl=eTransponderList::getInstance();
		if (!tl)
			return 0;
		eService *s = tl->searchService(service);
		if (!s) s = tl->searchSubService(service);
		return s;
	}
}

void eServiceHandlerDVB::removeRef(const eServiceReference &service)
{
	if ((service.data[0] < 0) || service.path)
	{
		cache.removeRef(service);
	}
}

int eServiceHandlerDVB::getPosition(int what)
{
#ifndef DISABLE_FILE
	if (!decoder)
		return -1;
	switch (what)
	{
	case posQueryLength:
		return decoder->getLength(0);
	case posQueryCurrent:
		return decoder->getPosition(0);
	case posQueryRealLength:
		return decoder->getLength(1);
	case posQueryRealCurrent:
		return decoder->getPosition(1);
	default:
		return -1;
	}
#else
	return -1;
#endif
}

eAutoInitP0<eServiceHandlerDVB> i_eServiceHandlerDVB(eAutoInitNumbers::service+2, "eServiceHandlerDVB");
