#include <lib/dvb/frontend.h>

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <cmath>

#include <lib/base/ebase.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/esection.h>
#include <lib/dvb/decoder.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/driver/rc.h>

// need this to check if currently a dvb service running.. 
// for lostlock/retune handling.. 
// when no dvb service is running or needed we call savePower..
#include <lib/dvb/service.h> 
// for check if in background a recording is running
#include <lib/dvb/edvb.h>
#include <lib/system/math.h>

eFrontend* eFrontend::frontend;

#ifndef I2C_SLAVE_FORCE
#define I2C_SLAVE_FORCE	0x0706
#endif

eFrontend::eFrontend(int type, const char *demod, const char *sec)
:type(type), 
	curRotorPos(10000), transponder(0), rotorTimer1(eApp), 
	rotorTimer2(eApp), 
#if HAVE_DVB_API_VERSION >= 3
	timeout(eApp), 
#endif
	checkRotorLockTimer(eApp), checkLockTimer(eApp), 
	updateTransponderTimer(eApp), sn(0), noRotorCmd(0),
	m_canBlindScan(false),agc1r(0x0C),agctrimmode(agctrimready),bermax(0),berave(0)
{
	init_eFrontend(type,demod,sec);
}
void eFrontend::init_eFrontend(int type, const char *demod, const char *sec)
{
	CONNECT(rotorTimer1.timeout, eFrontend::RotorStartLoop );
	CONNECT(rotorTimer2.timeout, eFrontend::RotorRunningLoop );
	CONNECT(checkRotorLockTimer.timeout, eFrontend::checkRotorLock );
	CONNECT(checkLockTimer.timeout, eFrontend::checkLock );
	CONNECT(updateTransponderTimer.timeout, eFrontend::updateTransponder );

#if HAVE_DVB_API_VERSION >= 3
	CONNECT(timeout.timeout, eFrontend::tuneFailed);
#endif

	fd=::open(demod, O_RDWR|O_NONBLOCK);
	if (fd<0)
	{
		type=eSystemInfo::feUnknown;
		fd=-1;
#if HAVE_DVB_API_VERSION < 3
		secfd=-1;
#endif
		perror(demod);
		return;
	}

#if HAVE_DVB_API_VERSION < 3
	FrontendEvent ev;
#else
	dvb_frontend_event ev;
#endif
	while ( !::ioctl(fd, FE_GET_EVENT, &ev) )
		eDebug("[FE] clear FE_EVENT queue");

	sn=new eSocketNotifier(eApp, fd, eSocketNotifier::Read);
	CONNECT(sn->activated, eFrontend::readFeEvent );
#if HAVE_DVB_API_VERSION < 3
	if (type==eSystemInfo::feSatellite)
	{
		secfd=::open(sec, O_RDWR);
		if (secfd<0)
		{
			perror(sec);
			return;
		}
	} else
	{
		secfd=-1;
		if (type == eSystemInfo::feTerrestrial)
		{
			int antenna_5v_disabled=0;
			eConfig::getInstance()->getKey("/elitedvb/DVB/config/disable_5V", antenna_5v_disabled);
			setTerrestrialAntennaVoltage(antenna_5v_disabled);
		}
	}
#else
	curContTone = curVoltage = -1;
#endif
	needreset = 2;
	
// check if tuner can blindscan
	if (type == eSystemInfo::feSatellite)
	{
		FILE *f=fopen("/proc/bus/nim_sockets", "rt");
		while (f)
		{
			char buffer[128];
			if (!fgets(buffer, 128, f))
			{
				fclose(f);
				break;
			}
			if (strstr(buffer, "Name:") && strstr(buffer, "Alps -S(STV0288)"))
			{
				m_canBlindScan=true;
				fclose(f);
				break;
			}
		}
	}
}

void eFrontend::checkLock()
{
	if (!Locked())
	{
		if (++lostlockcount > 2)
			setFrontend();
		else
			checkLockTimer.start(750,true);
	}
	else
	{
		lostlockcount=0;
		checkLockTimer.start(750,true);
	}
}

void eFrontend::checkRotorLock()
{
	if (!transponder)
		return;
	if (type==eSystemInfo::feSatellite)
	{
		eSatellite * sat = eTransponderList::getInstance()->findSatellite(transponder->satellite.orbital_position);
		if (sat)
		{
			eLNB *lnb = sat->getLNB();
			if (lnb && lnb->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 && curRotorPos>=11000 )
			{
				int snr = SNR();
				eDebug("[FE] checkRotorLock SNR is %d... old was %d", snr, curRotorPos-=11000 );
				if ( Locked() && abs(curRotorPos - snr) < 5000 )
				{
					eDebug("[FE] rotor has stopped..");
					curRotorPos=newPos;
					/*emit*/ tunedIn(transponder, 0);
//					eDebug("!!!!!!!!!!!!!!!! TUNED IN OK 1 !!!!!!!!!!!!!!!!");
					if ( !eDVB::getInstance()->getScanAPI() )
					{
						eDebug("[FE] start update transponder data timer");
						updateTransponderTimer.start(2000,true);
						checkLockTimer.start(750,true);
//						eDebug("ROTOR STOPPED 2");
						s_RotorStopped();
					}
					return;
				}
				else
				{
					eDebug("[FE] rotor already running");
					curRotorPos=11000;
					if ( eDVB::getInstance()->getScanAPI() )
					{
//						eDebug("!!!!!!!!!!!!!!!! TUNED IN EAGAIN 1 !!!!!!!!!!!!!!!!");
						/*emit*/ tunedIn(transponder, -EAGAIN);
					}
					else
						setFrontend();
					return;
				}
			}
		}
	}
}

int eFrontend::setFrontend()
{
	if (type == eSystemInfo::feUnknown)
		return -1;
	if (ioctl(fd, FE_SET_FRONTEND, &front)<0)
	{
		eDebug("FE_SET_FRONTEND failed (%m)");
		return -1;
	}
	eDebug("FE_SET_FRONTEND OK");
#if HAVE_DVB_API_VERSION >= 3
	// API V3 drivers have no working TIMEDOUT event.. 
	timeout.start(3000,true);   
#endif
	return 0;
}

void eFrontend::tuneOK()
{
#if HAVE_DVB_API_VERSION >= 3
	// stop userspace lock timeout
	timeout.stop();  
#endif
	if (!transponder || type == eSystemInfo::feUnknown)
		return;
	if (type==eSystemInfo::feSatellite)
	{
		eSatellite * sat = eTransponderList::getInstance()->findSatellite(transponder->satellite.orbital_position);
		if (sat)
		{
			eLNB *lnb = sat->getLNB();
			if (lnb && lnb->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2)
			{
				if (curRotorPos==5000) // rotor running in input power mode
				{
					eDebug("[FE] ignore .. rotor is running");
					return;
				}
				if (curRotorPos==11000)
				{
					curRotorPos+=SNR();
					checkRotorLockTimer.start(2000,true);   // check SNR in 2 sek
					eDebug("[FE] start check locktimer cur snr is %d", curRotorPos );
					return;
				}
			}
		}
	}
	if ( !eDVB::getInstance()->getScanAPI() )
	{
		eDebug("[FE] start update transponder data timer");
		updateTransponderTimer.start(2000,true);
		checkLockTimer.start(750,true);
	}
//	eDebug("!!!!!!!!!!!!!!!! TUNED IN OK 2 !!!!!!!!!!!!!!!!");
	/*emit*/ tunedIn(transponder, 0);
	return;
}

void eFrontend::tuneFailed()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return;
	if (type==eSystemInfo::feSatellite)
	{
		eSatellite * sat = eTransponderList::getInstance()->findSatellite(transponder->satellite.orbital_position);
		if (sat)
		{
			eLNB *lnb = sat->getLNB();
			if ( lnb && lnb->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 )
			{
				if (curRotorPos==5000) // rotor running in input power mode
				{
					eDebug("[FE] ignore .. rotor is running");
					return;
				}
				if ( curRotorPos==11000 )
				{
					eDebug("[FE] RotorPos uninitialized (%d)", tries);
					// check every transponder two times..
					if (eDVB::getInstance()->getScanAPI() && ++tries > 1)
					{
						tries=0;
						eDebug("[FE] don't set this TP to error");
//						eDebug("!!!!!!!!!!!!!!!! TUNED IN EAGAIN 2 !!!!!!!!!!!!!!!!");
						/*emit*/ tunedIn(transponder, -EAGAIN);  // nextTransponder
						return;
					}
					setFrontend();
					return;
				}
			}
		}
	}
//			eDebug("!!!!!!!!!!!!!!!! TUNED IN ETIMEDOUT 1 !!!!!!!!!!!!!!!!");						
	if ( !eDVB::getInstance()->getScanAPI() )
	{
		if (++lostlockcount > 5)
		{
			needreset=2;
			lostlockcount=0;
			transponder->tune();
		}
		else
			setFrontend();
	}
	/*emit*/ tunedIn(transponder, -ETIMEDOUT);
	return;
}

void eFrontend::readFeEvent(int what)
{
#if HAVE_DVB_API_VERSION < 3
	FrontendEvent ev;
	memset(&ev, 0, sizeof(FrontendEvent));
#else
	dvb_frontend_event ev;
	memset(&ev, 0, sizeof(dvb_frontend_event));
#endif
	if ( ::ioctl(fd, FE_GET_EVENT, &ev) < 0 )
	{
		eDebug("FE_GET_EVENT failed(%m)");
		return;
	}
	if ( !transponder )
	{
		eDebug("[FE] no transponder .. ignore FE Events");
		return;
	}
#if HAVE_DVB_API_VERSION < 3 
	switch (ev.type)
	{
		case FE_COMPLETION_EV:
#else
		if ( ev.status & FE_HAS_LOCK )
#endif
		{
			eDebug("[FE] evt. locked");
			tuneOK();
			return;
		}
#if HAVE_DVB_API_VERSION < 3
		case FE_FAILURE_EV:
		{
			eDebug("[FE] evt. failure");
			tuneFailed();
			return;
		}
#else
		else if ( ev.status & FE_TIMEDOUT )
			eDebug("[FE].");
#endif
#if HAVE_DVB_API_VERSION < 3
		default:
			eDebug("[FE] unhandled event (type %d)", ev.type);
	}
#else
		else if ( ev.status )
			eDebug("[FE] unhandled event (status %d)", ev.status);
#endif
}

void eFrontend::InitDiSEqC()
{
	lastcsw = lastSmatvFreq = lastRotorCmd = lastucsw = lastToneBurst = -1;
	lastLNB=0;
	transponder=0;
	std::list<eLNB> &lnbs = eTransponderList::getInstance()->getLNBs();
	for (std::list<eLNB>::iterator it(lnbs.begin()); it != lnbs.end(); ++it)
		if ( it->getDiSEqC().DiSEqCMode != eDiSEqC::NONE &&
			( it->getDiSEqC().DiSEqCParam != eDiSEqC::SENDNO ||
				it->getDiSEqC().uncommitted_cmd ) )
	{
		// DiSEqC Reset
		sendDiSEqCCmd( 0, 0 );
		// peripheral power supply on
		sendDiSEqCCmd( 0, 3 );
		usleep(150000);
		break;
	}
}

eFrontend::~eFrontend()
{
	delete sn;
	if (fd>=0)
		::close(fd);
#if HAVE_DVB_API_VERSION < 3
	if (secfd>=0)
		::close(secfd);
#endif
	frontend=0;
}

void eFrontend::getStatus(int& status, int& snrrel, eString& snrstring, int& agcrel, eString& agcstring, int& berrel, eString& berstring)
{
	status=Status(); 

	unsigned int temp;

	int db = 0;
	eConfig::getInstance()->getKey("/pli/SNRdB", db);

	temp=SNR();	
	if (db) 
	{
		temp=interpolateSNR(temp);
		snrrel=temp*100/ 180;
		snrstring=eString().sprintf("%.1f", (float)temp/10.0);
	}
	else
	{
		snrrel=temp*100/ 65535;
		snrstring=eString().sprintf("%d%%", temp*100/65535);
	}

	agcrel=SignalStrength()*100/65535;
	agcstring=eString().sprintf("%d%%",agcrel);

	int improvedBERenabled = 0;
	if (type == eSystemInfo::feSatellite)
		eConfig::getInstance()->getKey("/pli/ImprovedBER", improvedBERenabled);

	temp=BER();
	if (improvedBERenabled)
	{
		if( !(status & FE_HAS_LOCK) )
		{
			berave=0;
			agctrimmode=agctriminit;
		}
		else if ( temp<2 )
			if (berave==0)
				berave=6;
			else 
				berave=.75*berave+1.5;
		else if ( temp>200 || berave==0 )
			berave=log(2097152/temp)/2.302585;
		else
			berave=.75*berave+.25*log(2097152/temp)/2.302585;

		berrel=(int)(berave*100/6+.5);
		berstring=eString().sprintf("%.1f", berave);
	}
	else
	{
		if( temp<=0 )
			berrel=0;
		else
			berrel=(int)log2(temp);
		berstring=eString().sprintf("%d",temp);
	}

	int agctrimenabled = 0;
	if (type == eSystemInfo::feSatellite)
		eConfig::getInstance()->getKey("/pli/AGCtrim", agctrimenabled);

	if (agctrimenabled && improvedBERenabled && temp )	
	
	{
		bool agc1rchanged=false;	
		
		switch (agctrimmode)
		{
			case agctriminit:
				agc1r=0x0c;
				agctrimmode=agctrimfirst;
				agc1rchanged=true;
				break;
			case agctrimfirst:
				bermax=temp;
				agc1r--;
				agctrimmode=agctrimdownfirst;
				agc1rchanged=true;
				break;
			case agctrimdownfirst:
				if (temp<bermax)
				{
					bermax=temp;
					agc1r--;
					agctrimmode=agctrimdown;
					agc1rchanged=true;
				}
				else
				{
					agc1r+=2;
					agctrimmode=agctrimup;
					agc1rchanged=true;
				}
				break;
			case agctrimdown:
				if (temp<bermax)
				{
					bermax=temp;
					if ((agc1r & 0x3f ) > 6)
					{
						agc1r--;
						agc1rchanged=true;
					}
					else	
						agctrimmode=agctrimready;
				}
				else
				{
					agc1r++;
					agctrimmode=agctrimready;
					agc1rchanged=true;
				}
				break;
				case agctrimup:
				if (temp<bermax)
				{
					bermax=temp;
					if ((agc1r & 0x3f ) < 0x0f)
					{
						agc1r++;
						agc1rchanged=true;
					}
					else	
						agctrimmode=agctrimready;
				}
				else
				{
					agc1r--;
					agctrimmode=agctrimready;
					agc1rchanged=true;
				}
				break;
			case agctrimready:
				break;
		}

		if (agc1rchanged) 
		{
			writeI2CReg(0x68,0x0f,agc1r);
			eDebug("[Frontend] agc1reg mode=%i temp=%i bermax=%i agc1r=%i",agctrimmode, temp, bermax, agc1r);
		}
	}
}

void eFrontend::setBERMode (unsigned int mode)
{
	if (mode) 
	{  
		writeI2CReg(0x68,0x34,0x03);
	}
	else
	{
		writeI2CReg(0x68,0x34,0x93);
	}
}

void eFrontend::writeI2CReg (unsigned char i2cadres, unsigned char reg, unsigned char val)
{
	unsigned char buf[2];
	int fd = ::open("/dev/i2c/0",O_RDWR);

	buf[0] = reg;
	buf[1] = val;
	if(fd>=0)
	{
		int res=0;
		if(ioctl(fd,I2C_SLAVE_FORCE,i2cadres)>=0)
			res=::write(fd,buf,2); // register is in buf[0], value in buf[1]
		::close(fd);
		if(res == 2) return;
	}
	eDebug("[Frontend] Unable to write I2C Port");
}

int eFrontend::Status()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return 0;
#if HAVE_DVB_API_VERSION < 3
	FrontendStatus status=0;
#else
	fe_status_t status;
#endif
	if ( ioctl(fd, FE_READ_STATUS, &status) < 0 && errno != ERANGE )
		eDebug("FE_READ_STATUS failed (%m)");
	return status;
}
 
uint32_t eFrontend::BER()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return 0;
	uint32_t ber=0;
	if (ioctl(fd, FE_READ_BER, &ber) < 0 && errno != ERANGE)
		eDebug("FE_READ_BER failed (%m)");
	return ber;
}

int eFrontend::SignalStrength()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return 0;
	uint16_t strength=0;
	if (ioctl(fd, FE_READ_SIGNAL_STRENGTH, &strength) < 0 && errno != ERANGE)
		eDebug("FE_READ_SIGNAL_STRENGTH failed (%m)");
#if 0
	if ((strength<0) || (strength>65535))
	{
		eWarning("buggy SignalStrength driver (or old version) (%08x)", strength);
		strength=0;
	}
#endif
	return strength;
}

const int eFrontend::snr_table[19] =
{
	0,20000,36000,41000,44000,46000,47500,49000,50000,51000,
	52500,54000,55500,57000,58500,60000,61500,63000,65535
};

int eFrontend::interpolateSNR(const int snr)
{
	int tenth, i;

	for (i = 0; i < sizeof(snr_table) / sizeof(int), snr_table[i] <= snr; i++);
	i--;
	tenth = (10 * (snr - snr_table[i])) / (snr_table[i + 1] - snr_table[i]);
	return (i * 10) + tenth;
}

int eFrontend::SNR()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return 0;
	uint16_t snr=0;
	if (ioctl(fd, FE_READ_SNR, &snr) < 0 && errno != ERANGE)
		eDebug("FE_READ_SNR failed (%m)");
#if 0
	if ((snr<0) || (snr>65535))
	{
		eWarning("buggy SNR driver (or old version) (%08x)", snr);
		snr=0;
	}
#endif
	return snr;
}

int eFrontend::getSNR()
{
	int db = 0;
	eConfig::getInstance()->getKey("/pli/SNRdB", db);

	if (db)
		return interpolateSNR(SNR()) * 100 / 180;
	else
		return SNR() * 100 / 65535;
}

eString eFrontend::getSNRString()
{
	int db = 0;
	eConfig::getInstance()->getKey("/pli/SNRdB", db);

	if (db)
		return eString().sprintf("%.1f", (float) interpolateSNR(SNR()) / 10.0);
	else
		return eString().sprintf("%d%%", SNR() * 100 / 65535);
}

uint32_t eFrontend::UncorrectedBlocks()
{
	if (!transponder || type == eSystemInfo::feUnknown)
		return 0;
	uint32_t ublocks=0;
	if (ioctl(fd, FE_READ_UNCORRECTED_BLOCKS, &ublocks) < 0 && errno != ERANGE)
		eDebug("FE_READ_UNCORRECTED_BLOCKS failed (%m)");
	return ublocks;
}

static CodeRate etsiToDvbApiFEC(int fec)
{
	switch (fec)
	{
	case -1:
	case 15:
		return FEC_NONE;
	case 0:
		return FEC_AUTO;
	case 1:
		return FEC_1_2;
	case 2:
		return FEC_2_3;
	case 3:
		return FEC_3_4;
	case 4:
		return FEC_5_6;
	case 5:
		return FEC_7_8;
	default:
		break;
	}
	return FEC_AUTO;
}

static Modulation etsiToDvbApiModulation(int mod)
{
	switch (mod)
	{
	case 1:
		return QAM_16;
	case 2:
		return QAM_32;
	case 3:
		return QAM_64;
	case 4:
		return QAM_128;
	case 5:
		return QAM_256;
	default:
		return QAM_64;
	}
}

// conversions etsi -> API for DVB-T
static Modulation etsiToDvbApiConstellation(int mod)
{
	switch (mod)
	{
		case 0:
			return QPSK;
		case 1:
			return QAM_16;
		case 2:
			return QAM_64;
		default:
			break;
	}
	return (Modulation)QAM_AUTO;
}

static CodeRate etsiToDvbApiCodeRate(int rate)
{
	switch(rate)
	{
		case 0:
			return FEC_1_2;
		case 1:
			return FEC_2_3;
		case 2:
			return FEC_3_4;
		case 3:
			return FEC_5_6;
		case 4:
			return FEC_7_8;
		default:
			break;
	}
	return (CodeRate)FEC_AUTO;
}

static BandWidth etsiToDvbApiBandwidth(int bwidth)
{
	switch(bwidth)
	{
		case 0:
			return BANDWIDTH_8_MHZ;
		case 1:
			return BANDWIDTH_7_MHZ;
		case 2:
			return BANDWIDTH_6_MHZ;
		default:
			break;
	}
	return (BandWidth)BANDWIDTH_AUTO;
}

static GuardInterval etsiToDvbApiGuardInterval( int interval )
{
	switch(interval)
	{
		case 0:
			return GUARD_INTERVAL_1_32;
		case 1:
			return GUARD_INTERVAL_1_16;
		case 2:
			return GUARD_INTERVAL_1_8;
		case 3:
			return GUARD_INTERVAL_1_4;
		default:
			break;
	}
	return (GuardInterval)GUARD_INTERVAL_AUTO;
}

static TransmitMode etsiToDvbApiTransmitMode( int mode )
{
	switch(mode)
	{
		case 0:
			return TRANSMISSION_MODE_2K;
		case 1:
			return TRANSMISSION_MODE_8K;
		default:
			break;
	}
	return (TransmitMode)TRANSMISSION_MODE_AUTO;
}

static Hierarchy etsiToDvbApiHierarchyInformation( int hierarchy )
{
	switch(hierarchy)
	{
		case 0:
			return HIERARCHY_NONE;
		case 1:
			return HIERARCHY_1;
		case 2:
			return HIERARCHY_2;
		case 3:
			return HIERARCHY_4;
		default:
			break;
	}
	return (Hierarchy)HIERARCHY_AUTO;
}

static int dvbApiToEtsiCodeRate(CodeRate cr)
{
	switch (cr) {
	case FEC_1_2:
		return 0;
	case FEC_2_3:
		return 1;
	case FEC_3_4:
		return 2;
	case FEC_5_6:
		return 3;
	case FEC_7_8:
		return 4;
	default:
		break;
	}
	return -1;
}

static int dvbApiToEtsiConstellation(Modulation mod)
{
	switch(mod)
	{
		case QPSK:
			return 0;
		case QAM_16:
			return 1;
		case QAM_64:
			return 2;
		default:
			break;
	}
	return 3;
}

static int dvbApiToEtsiBandWidth(BandWidth bw)
{
	switch(bw)
	{
		case BANDWIDTH_8_MHZ:
			return 0;
		case BANDWIDTH_7_MHZ:
			return 1;
		case BANDWIDTH_6_MHZ:
			return 2;
		default:
			break;
	}
	return 3;
}

static int dvbApiToEtsiGuardInterval( GuardInterval interval )
{
	switch(interval)
	{
		case GUARD_INTERVAL_1_32:
			return 0;
		case GUARD_INTERVAL_1_16:
			return 1;
		case GUARD_INTERVAL_1_8:
			return 2;
		case GUARD_INTERVAL_1_4:
			return 3;
		default:
			break;
	}
	return 4;
}

static int dvbApiToEtsiTransmitMode( TransmitMode mode )
{
	switch(mode)
	{
		case TRANSMISSION_MODE_2K:
			return 0;
		case TRANSMISSION_MODE_8K:
			return 1;
		default:
			break;
	}
	return 2;
}

static int dvbApiToEtsiHierarchyInformation( Hierarchy hierarchy )
{
	switch(hierarchy)
	{
		case HIERARCHY_NONE:
			return 0;
		case HIERARCHY_1:
			return 1;
		case HIERARCHY_2:
			return 2;
		case HIERARCHY_4:
			return 3;
		default:
			break;
	}
	return 4;
}


int gotoXTable[10] = { 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x0A, 0x0B, 0x0D, 0x0E };

int eFrontend::readInputPower()
{
	int power=0;
	if ( eSystemInfo::getInstance()->canMeasureLNBCurrent() )
	{
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::DM7000:
			case eSystemInfo::DM7020:
			{
				// open front prozessor
				int fp=::open("/dev/dbox/fp0", O_RDWR);
				if (fp < 0)
				{
					eDebug("couldn't open fp");
					return -1;
				}
#define FP_IOCTL_GET_ID 0
				static bool old_fp = (::ioctl(fp, FP_IOCTL_GET_ID) < 0);
				if ( ioctl( fp, old_fp ? 9 : 0x100, &power ) < 0 )
				{
					eDebug("FP_IOCTL_GET_LNB_CURRENT failed (%m)");
					return -1;
				}
				::close(fp);  
				break;
			}
			default:
				eDebug("Inputpower read for platform %d not yet implemented", eSystemInfo::getInstance()->getHwType());
		}
	}
	return power;
}

#if HAVE_DVB_API_VERSION < 3
int eFrontend::sendDiSEqCCmd( int addr, int Cmd, eString params, int frame )
{
	if (type != eSystemInfo::feSatellite)
		return -1;

	secCmdSequence seq;
	secCommand cmd;

	int cnt=0;
	for ( unsigned int i=0; i < params.length() && i < 6; i+=2 )
		cmd.u.diseqc.params[cnt++] = strtol( params.mid(i, 2).c_str(), 0, 16 );
    
	cmd.type = SEC_CMDTYPE_DISEQC_RAW;
	cmd.u.diseqc.cmdtype = frame;
	cmd.u.diseqc.addr = addr;
	cmd.u.diseqc.cmd = Cmd;
	cmd.u.diseqc.numParams = cnt;

// debug output..
#if 0
	eString parms;
	for (int i=0; i < cnt; i++)
		parms+=eString().sprintf("0x%02x ",cmd.u.diseqc.params[i]);
#endif

	if ( transponder && lastLNB )
	{
//		eDebug("hold current voltage and continuous tone");
		// set Continuous Tone ( 22 Khz... low - high band )
		if ( transponder->satellite.frequency > lastLNB->getLOFThreshold() )
			seq.continuousTone = SEC_TONE_ON;
		else 
			seq.continuousTone = SEC_TONE_OFF;
		// set voltage
		if ( transponder->satellite.polarisation == polVert )
			seq.voltage = SEC_VOLTAGE_13;
		else
			seq.voltage = SEC_VOLTAGE_18;
	}
	else
	{
		eDebug("set continuous tone OFF and voltage to 13V");
		seq.continuousTone = SEC_TONE_OFF;
		seq.voltage = SEC_VOLTAGE_13;
	}
    
//  eDebug("cmdtype = %02x, addr = %02x, cmd = %02x, numParams = %02x, params=%s", frame, addr, Cmd, cnt, parms.c_str() );
	seq.miniCommand = SEC_MINI_NONE;
	seq.commands=&cmd;
	seq.numCommands=1;

	if ( ioctl(secfd, SEC_SEND_SEQUENCE, &seq) < 0 )
	{
		eDebug("SEC_SEND_SEQUENCE failed ( %m )");
		return -1;
	}
/*  else
		eDebug("cmd send");*/

	lastcsw = lastucsw = -1;

	return 0;
}
#else
int eFrontend::sendDiSEqCCmd( int addr, int Cmd, eString params, int frame )
{
	if (type != eSystemInfo::feSatellite)
		return -1;
	struct dvb_diseqc_master_cmd cmd;

	cmd.msg[0]=frame;
	cmd.msg[1]=addr;
	cmd.msg[2]=Cmd;
	cmd.msg_len=3;

	for (uint8_t i = 0; i < params.length() && i < 6; i += 2)
		cmd.msg[cmd.msg_len++] = strtol( params.mid(i, 2).c_str(), 0, 16 );

	if ( curVoltage == -1 && (::ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) < 0) )
		eDebug("FE_SET_VOLTAGE (18) failed (%m)");

	if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) < 0)
	{
		eDebug("FE_SET_TONE failed (%m)");
		return -1;
	}
	curContTone = eSecCmdSequence::TONE_OFF;

	if (ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd) < 0)
	{
		eDebug("FE_DISEQC_SEND_MASTER_CMD failed (%m)");
		return -1;
	}

	lastcsw = lastucsw = -1;

	return 0;
}
#endif

#if HAVE_DVB_API_VERSION < 3
int eFrontend::SendSequence( const eSecCmdSequence &s )
{
	if (type != eSystemInfo::feSatellite)
		return -1;
	secCmdSequence seq;
	memset( &seq, 0, sizeof(seq) );
	seq.commands = s.commands;
	seq.numCommands = s.numCommands;
	seq.continuousTone = s.continuousTone;
	seq.miniCommand = s.toneBurst;
	switch ( s.voltage )
	{
		case eSecCmdSequence::VOLTAGE_13:
			seq.voltage=s.increasedVoltage?SEC_VOLTAGE_13_5:SEC_VOLTAGE_13;
			break;
		case eSecCmdSequence::VOLTAGE_18:
			seq.voltage=s.increasedVoltage?SEC_VOLTAGE_18_5:SEC_VOLTAGE_18;
			break;
		case eSecCmdSequence::VOLTAGE_OFF:
		default:
			seq.voltage=SEC_VOLTAGE_OFF;
			break;
	}
	return ::ioctl(secfd, SEC_SEND_SEQUENCE, &seq);
}
#else
int eFrontend::SendSequence( const eSecCmdSequence &seq )
{
	if (type != eSystemInfo::feSatellite)
		return -1;

	int i=0, ret=0, wait=0;
	dvb_diseqc_master_cmd *scommands;

// set Tone
//	eDebug("curContTone = %d, newTone = %d", curContTone, seq.continuousTone );
	if ( curContTone != eSecCmdSequence::TONE_OFF &&
		( seq.continuousTone == eSecCmdSequence::TONE_OFF
	// diseqc command or minidiseqc command to sent ?
		|| seq.numCommands || seq.toneBurst != eSecCmdSequence::NONE ) )
	{
//		eDebug("disable cont Tone");
		ret = ioctl(fd, FE_SET_TONE, SEC_TONE_OFF);
		if ( ret < 0 )
		{
			eDebug("FE_SET_TONE failed (%m)");
			return ret;
		}
		curContTone = eSecCmdSequence::TONE_OFF;
		wait=1;
	}

	if (seq.increasedVoltage)
	{
//		eDebug("enable high voltage");
		ret = ioctl(fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 1);
		if ( ret < 0 )
		{
			eDebug("FE_ENABLE_HIGH_LNB_VOLTAGE failed (%m)");
			return ret;
		}
	} 
//	eDebug("curVoltage = %d, new Voltage = %d", curContTone, seq.voltage);
	if ( seq.voltage != curVoltage )
	{
		ret = ioctl(fd, FE_SET_VOLTAGE, seq.voltage);
		if (  ret < 0 )
		{
			eDebug("FE_SET_VOLTAGE failed (%m)");
			return ret;
		}
		curVoltage = seq.voltage;
//		eDebug("set voltage to %s", seq.voltage==eSecCmdSequence::VOLTAGE_13?"13 V":seq.voltage==eSecCmdSequence::VOLTAGE_18?"18 V":"unknown ... bug!!");
		wait=1;
	}

	if ( wait )
	{
//		eDebug("delay 30 ms");
		usleep(30*1000);
		wait=0;
	}

	while (true && seq.numCommands)
	{
		scommands = &seq.commands[i];

		ret = ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, scommands);
		if ( ret < 0)
		{
			eDebug("FE_DISEQC_SEND_MASTER_CMD failed (%m)");
			return ret;
		}
//		eDebug("DiSEqC Cmd sent %02x %02x %02x %02x", scommands->msg[0], scommands->msg[1], scommands->msg[2], scommands->msg[3] );
/*
		if ( scommands->msg[0] & 1 )
		{
			if ( scommands->msg[2] == 0x39 )
			{
				eDebug("(%d)uncommited repeat command sent", i);
			}
			else
			{
				eDebug("(%d)commited repeat command sent", i);
			}
		}
		else
		{
			if ( scommands->msg[2] == 0x39 )
			{
				eDebug("(%d)uncommited command sent", i);
			}
			else
			{
				eDebug("(%d)commited command sent", i);
			}
		}*/

		i++;
		if ( i < seq.numCommands )  // another command is to send...
		{
			if (wait && seq.commands[i].msg[0] & 1)
			{
				usleep(wait*1000);
//				eDebug("delay %dms",wait);
			}
			if ( seq.commands[i].msg[2] == scommands->msg[2]
						&& seq.commands[i].msg[3] == scommands->msg[3] )
			// send repeat without uncommitted switch in gap... wait 120 ms
			{
				usleep(120*1000);
//				eDebug("delay 120ms");
			}
			else if ( seq.commands[i].msg[0] & 1  // repeat
				|| (i+1 < seq.numCommands &&
				seq.commands[i-1].msg[2] == seq.commands[i+1].msg[2] ))
			{
				if (!wait)
				{
					// we calc prev message length
					int mlength = (scommands->msg_len * 8 + scommands->msg_len) * 15;
//					eDebug("messageLength = %dms", mlength / 10 );
					wait = ( ( (1200 - mlength) / 20) );
					// half time we wait before next cmd
					usleep(wait*1000);
//					eDebug("delay %dms",wait);
				}
				else
					wait = 0;
			}  // MSG[1] ??? or MSG
			else if ( seq.commands[i].msg[1] == 0x58 ) // handle SMATV Cmd
			{
//				eDebug("delay 10ms");  // Standard > 6ms
				usleep( 10*1000 );
			}
			else  // we calc the delay between two cmds with difference types.. switch..rotor
			{
				usleep( 120*1000 );  // wait after msg
//				eDebug("delay 120ms");
				wait=0;
			}
		}
		else
			break;
	}

	if (seq.numCommands)  // When DiSEqC Commands was sent.. then wait..
	{
//		eDebug("delay 30ms after send last diseqc command");  // Standard > 15ms
		usleep(30*1000);
	}

  // send minidiseqc
	if (seq.toneBurst != eSecCmdSequence::NONE)
	{
		ret = ioctl(fd, FE_DISEQC_SEND_BURST, seq.toneBurst);
		if ( ret < 0)
		{
			eDebug("FE_DISEQC_SEND_BURST failed (%m)");
			return ret;
		}
//		eDebug("toneBurst sent\n");

		usleep(30*1000); // after send toneburst we wait...
//		eDebug("delay 30ms");
	}

//	eDebug("curTone = %d, newTone = %d", curContTone, seq.continuousTone );
	if ( seq.continuousTone == eSecCmdSequence::TONE_ON && curContTone != SEC_TONE_ON )
	{
//		eDebug("enable continuous Tone");
		ret = ioctl(fd, FE_SET_TONE, SEC_TONE_ON);
		if ( ret < 0 )
		{
			eDebug("FE_SET_TONE failed (%m)");
			return ret;
		}
		else
		{
			curContTone = eSecCmdSequence::TONE_ON;
			usleep(10*1000);
//			eDebug("delay 10ms");
		}
	}
/////////////////////////////////////////
	return 0;
}
#endif

int eFrontend::RotorUseTimeout(eSecCmdSequence& seq, eLNB *lnb )
{
#if HAVE_DVB_API_VERSION < 3
	secCommand *commands = seq.commands;
#else
	dvb_diseqc_master_cmd *commands = seq.commands;
#endif
	// we send first the normal DiSEqC Switch Cmds
	// and then the Rotor CMD
	seq.numCommands--;

	int secTone = seq.continuousTone;
	// send DiSEqC Sequence ( normal diseqc switches )
	seq.continuousTone = eSecCmdSequence::TONE_OFF;
	if ( SendSequence(seq) < 0 )
	{
		eDebug("SendSequence failed (%m)");
		return -1;
	}
	else if ( lnb->getDiSEqC().SeqRepeat )   // Sequence Repeat selected ?
	{
		usleep( 100000 ); // between seq repeats we wait 75ms
		SendSequence(seq);  // then repeat the command
	}
	if ( lastLNB != lnb )
		usleep( 1000000 ); // wait 1sek
	else
		usleep( 100000 ); // wait 100ms

	// send DiSEqC Sequence (Rotor)
	seq.commands=&commands[seq.numCommands];  // last command is rotor cmd... see above...
	seq.numCommands=1;  // only rotor cmd
	seq.toneBurst = eSecCmdSequence::NONE;
	seq.continuousTone=secTone;

	if ( SendSequence(seq) < 0 )
	{
		eDebug("SendSequence failed (%m)");
		return -1;
	}
	else
		eDebug("Rotor Cmd is sent");

	tries=0;
	return 0;
}

int eFrontend::RotorUseInputPower(eSecCmdSequence& seq, eLNB *lnb )
{
#if HAVE_DVB_API_VERSION < 3
	secCommand *commands = seq.commands;
#else
	dvb_diseqc_master_cmd *commands = seq.commands;
#endif
	idlePowerInput=0;
	runningPowerInput=0;
	int secTone = seq.continuousTone;

	// we send first the normal DiSEqC Switch Cmds
	// and then the Rotor CMD
	seq.numCommands--;

//	eDebug("sent normal diseqc switch cmd");
//	eDebug("0x%02x,0x%02x,0x%02x,0x%02x, numParams=%d, numcmds=%d", seq.commands[0].u.diseqc.cmdtype, seq.commands[0].u.diseqc.addr, seq.commands[0].u.diseqc.cmd, seq.commands[0].u.diseqc.params[0], seq.numCommands, seq.commands[0].u.diseqc.numParams );
	// send DiSEqC Sequence ( normal diseqc switches )

	seq.continuousTone=SEC_TONE_OFF;
	if ( SendSequence(seq) < 0 )
	{
		eDebug("SendSequence failed (%m)");
		return -2;
	}
	else if ( lnb->getDiSEqC().SeqRepeat )   // Sequence Repeat selected ?
	{
		usleep( 100000 ); // between seq repeats we wait 100ms
		SendSequence(seq);  // then repeat the cmd
	}

	if ( lastLNB != lnb )
	{
		usleep( 1000*1000 ); // wait 1sek
//		eDebug("sleep 1sek");
	}
	else
	{
		usleep( 100*1000 ); // wait 100ms
//		eDebug("sleep 100ms");
	}

// get power input of Rotor on idle  not work on dbox yet .. only dreambox
	idlePowerInput = readInputPower();
	if ( idlePowerInput < 0 )
		return idlePowerInput;
// eDebug("idle power input = %dmA", idlePowerInput );

	// send DiSEqC Sequence (Rotor)
	seq.commands=&commands[seq.numCommands];  // last command is rotor cmd... see above...
	seq.numCommands=1;  // only rotor cmd
	seq.toneBurst = eSecCmdSequence::NONE;

//	eDebug("0x%02x,0x%02x,0x%02x,0x%02x,0x%02x", seq.commands[0].u.diseqc.cmdtype, seq.commands[0].u.diseqc.addr, seq.commands[0].u.diseqc.cmd, seq.commands[0].u.diseqc.params[0], seq.commands[0].u.diseqc.params[1]);
	seq.continuousTone=secTone;
	if ( SendSequence(seq) < 0 )
	{
		eDebug("SendSequence failed (%m)");
		return -2;
	}
	// set rotor start timeout  // 2 sek..
	gettimeofday(&rotorTimeout,0);
	rotorTimeout+=2000;
	RotorStartLoop();
	return 0;
}

void eFrontend::RotorStartLoop()
{
	// timeouted ??
	timeval now;
	gettimeofday(&now,0);
	if ( rotorTimeout < now )
	{
		eDebug("rotor has timeoutet :( ");
		/* emit */ s_RotorTimeout();
		RotorFinish();
	}
	else
	{
		runningPowerInput = readInputPower();
		if ( runningPowerInput < 0 )
			return;
//		eDebug("running %d mA", runningPowerInput);
//		eDebug("delta %d mA", DeltaA);

		if ( abs(runningPowerInput-idlePowerInput ) >= (DeltaA&0xFF) ) // rotor running ?
		{
			if ( (DeltaA & 0x200) == 0x200 )
			{
				eDebug("Rotor is Running");

				if ( !eDVB::getInstance()->getScanAPI() )
				{
//					eDebug("ROTOR RUNNING EMIT");
					/* emit */ s_RotorRunning( newPos );
				}

				// set rotor running timeout  // 360 sek
				gettimeofday(&rotorTimeout,0);
				rotorTimeout+=360000;
				RotorRunningLoop();
				return;
			}
			else
				DeltaA+=0x100;
		}
		else
			DeltaA &= ~0xF00;
		rotorTimer1.start(50,true);  // restart timer
	}
}

void eFrontend::RotorRunningLoop()
{
	timeval now;
	gettimeofday(&now,0);
	if ( rotorTimeout < now )
	{
		eDebug("Rotor timeouted :-(");
		/* emit */ s_RotorTimeout();
	}
	else
	{
		runningPowerInput = readInputPower();
		if ( runningPowerInput < 0 )
			return;
//		eDebug("running %d mA", runningPowerInput);

		if ( abs( idlePowerInput-runningPowerInput ) <= (DeltaA&0xFF) ) // rotor stoped ?
		{
			if ( (DeltaA & 0x200) == 0x200 )
			{
				eDebug("Rotor Stopped");
				/* emit */ s_RotorStopped();
				RotorFinish();
				return;
			}
			else
				DeltaA+=0x100;
		}
		else
			DeltaA &= ~0xF00;
		rotorTimer2.start(50,true);  // restart timer
	}
}

void eFrontend::RotorFinish(bool tune)
{
	if (type != eSystemInfo::feSatellite)
		return;
	if ( eSystemInfo::getInstance()->canMeasureLNBCurrent() == 1 )
	{
		if ( voltage != eSecCmdSequence::VOLTAGE_18 )
#if HAVE_DVB_API_VERSION < 3
			if (ioctl(secfd, SEC_SET_VOLTAGE, increased ? SEC_VOLTAGE_13_5 : SEC_VOLTAGE_13) < 0 )
				eDebug("SEC_SET_VOLTAGE failed (%m)");
#else
		if ( ioctl(fd, FE_SET_VOLTAGE, voltage) < 0 )
			eDebug("FE_SET_VOLTAGE failed (%m)");
		curVoltage = voltage;
#endif
	}
	else  // can only measure with lower lnb voltage ( 13V )
	{
		if ( voltage != eSecCmdSequence::VOLTAGE_13 )
#if HAVE_DVB_API_VERSION < 3
			if (ioctl(secfd, SEC_SET_VOLTAGE, increased ? SEC_VOLTAGE_18_5 : SEC_VOLTAGE_18) < 0 )
				eDebug("SEC_SET_VOLTAGE failed (%m)");
#else
		 if ( ioctl(fd, FE_SET_VOLTAGE, voltage) < 0 )
			eDebug("FE_SET_VOLTAGE failed (%m)");
		curVoltage = voltage;
#endif
	}
	curRotorPos=newPos;
	if ( tune && transponder )
		transponder->tune();
}

double calcElevation( double SatLon, double SiteLat, double SiteLon, int Height_over_ocean = 0 )
{
	double  a0=0.58804392,
					a1=-0.17941557,
					a2=0.29906946E-1,
					a3=-0.25187400E-2,
					a4=0.82622101E-4,

					f = 1.00 / 298.257, // Earth flattning factor

					r_sat=42164.57, // Distance from earth centre to satellite

					r_eq=6378.14,  // Earth radius

					sinRadSiteLat=SIN(Radians(SiteLat)),
					cosRadSiteLat=COS(Radians(SiteLat)),

					Rstation = r_eq / ( std::sqrt( 1.00 - f*(2.00-f)*sinRadSiteLat*sinRadSiteLat ) ),

					Ra = (Rstation+Height_over_ocean)*cosRadSiteLat,
					Rz= Rstation*(1.00-f)*(1.00-f)*sinRadSiteLat,

					alfa_rx=r_sat*COS(Radians(SatLon-SiteLon)) - Ra,
					alfa_ry=r_sat*SIN(Radians(SatLon-SiteLon)),
					alfa_rz=-Rz,

					alfa_r_north=-alfa_rx*sinRadSiteLat + alfa_rz*cosRadSiteLat,
					alfa_r_zenith=alfa_rx*cosRadSiteLat + alfa_rz*sinRadSiteLat,

					El_geometric=Deg(ATAN( alfa_r_zenith/std::sqrt(alfa_r_north*alfa_r_north+alfa_ry*alfa_ry))),

					x = std::fabs(El_geometric+0.589),
					refraction=std::fabs(a0+a1*x+a2*x*x+a3*x*x*x+a4*x*x*x*x),
          El_observed = 0.00;

	if (El_geometric > 10.2)
		El_observed = El_geometric+0.01617*(COS(Radians(std::fabs(El_geometric)))/SIN(Radians(std::fabs(El_geometric))) );
	else
	{
		El_observed = El_geometric+refraction ;
	}

	if (alfa_r_zenith < -3000)
		El_observed=-99;

	return El_observed;
}

double calcAzimuth(double SatLon, double SiteLat, double SiteLon, int Height_over_ocean=0)
{
	double	f = 1.00 / 298.257, // Earth flattning factor

					r_sat=42164.57, // Distance from earth centre to satellite

					r_eq=6378.14,  // Earth radius

					sinRadSiteLat=SIN(Radians(SiteLat)),
					cosRadSiteLat=COS(Radians(SiteLat)),

					Rstation = r_eq / ( std::sqrt( 1 - f*(2-f)*sinRadSiteLat*sinRadSiteLat ) ),
					Ra = (Rstation+Height_over_ocean)*cosRadSiteLat,
					Rz = Rstation*(1-f)*(1-f)*sinRadSiteLat,

					alfa_rx = r_sat*COS(Radians(SatLon-SiteLon)) - Ra,
					alfa_ry = r_sat*SIN(Radians(SatLon-SiteLon)),
					alfa_rz = -Rz,

					alfa_r_north = -alfa_rx*sinRadSiteLat + alfa_rz*cosRadSiteLat,

					Azimuth = 0.00;

	if (alfa_r_north < 0)
		Azimuth = 180+Deg(ATAN(alfa_ry/alfa_r_north));
	else
		Azimuth = Rev(360+Deg(ATAN(alfa_ry/alfa_r_north)));

	return Azimuth;
}

double calcDeclination( double SiteLat, double Azimuth, double Elevation)
{
	return Deg( ASIN(SIN(Radians(Elevation)) *
												SIN(Radians(SiteLat)) +
												COS(Radians(Elevation)) *
												COS(Radians(SiteLat)) +
												COS(Radians(Azimuth))
												)
						);
}

double calcSatHourangle( double Azimuth, double Elevation, double Declination, double Lat )
{
	double a = - COS(Radians(Elevation)) *
							 SIN(Radians(Azimuth)),

				 b = SIN(Radians(Elevation)) *
						 COS(Radians(Lat)) -
						 COS(Radians(Elevation)) *
						 SIN(Radians(Lat)) *
						 COS(Radians(Azimuth)),

// Works for all azimuths (northern & sourhern hemisphere)
						 returnvalue = 180 + Deg(ATAN(a/b));

	(void)Declination;

	if ( Azimuth > 270 )
	{
		returnvalue = ( (returnvalue-180) + 360 );
		if (returnvalue>360)
			returnvalue = 360 - (returnvalue-360);
  }

	if ( Azimuth < 90 )
		returnvalue = ( 180 - returnvalue );

	return returnvalue;
}

void eFrontend::updateTransponder()
{
	if (type == eSystemInfo::feUnknown)
		return;
	if (!transponder)
		return;
	if (!eSystemInfo::getInstance()->canUpdateTransponder())
		return;
	if (!Locked())
		return;

#if HAVE_DVB_API_VERSION < 3
	FrontendParameters front;
#else
	struct dvb_frontend_parameters front;
#endif
	if (ioctl(fd, FE_GET_FRONTEND, &front)<0) {
		eDebug("FE_GET_FRONTEND (%m)");
		return;
	}

	if ((type==eSystemInfo::feSatellite) && (transponder->satellite.valid))
	{
		unsigned int freq, inv, sr;
#if HAVE_DVB_API_VERSION < 3
		freq = front.Frequency;
		inv = front.Inversion;
		sr = front.u.qpsk.SymbolRate;
#else
		freq = front.frequency;
		inv = front.inversion;
		sr = front.u.qpsk.symbol_rate;
#endif
		eDebug("[FE] update transponder data");
		eSatellite * sat = eTransponderList::getInstance()->findSatellite(transponder->satellite.orbital_position);
		if (sat)
		{
			eLNB *lnb = sat->getLNB();
			if (lnb)
			{
				int lof = transponder->satellite.frequency > lnb->getLOFThreshold() ?
					lnb->getLOFHi() : lnb->getLOFLo();
				int tuned_freq = transponder->satellite.frequency - lof;
				if ( tuned_freq < 0 ) // c-band
					transponder->satellite.frequency = abs(freq-lof);
				else
					transponder->satellite.frequency = freq+lof;
			}
		}
/*		transponder->satellite.fec = front.u.qpsk.FEC_inner; */
		transponder->satellite.inversion = inv;
		transponder->satellite.symbol_rate = sr;
	}
	else if ((type==eSystemInfo::feCable) && (transponder->cable.valid)) {
#if HAVE_DVB_API_VERSION < 3
		// FIXME
#else
		// FIXME
#endif
	}
	else if ((type==eSystemInfo::feTerrestrial) && (transponder->terrestrial.valid)) {
#if HAVE_DVB_API_VERSION < 3
		transponder->terrestrial.centre_frequency = front.Frequency;
		transponder->terrestrial.bandwidth = dvbApiToEtsiBandWidth(front.u.ofdm.bandWidth);
		transponder->terrestrial.code_rate_hp = dvbApiToEtsiCodeRate(front.u.ofdm.HP_CodeRate);
		transponder->terrestrial.code_rate_lp = dvbApiToEtsiCodeRate(front.u.ofdm.LP_CodeRate);
		transponder->terrestrial.constellation = dvbApiToEtsiConstellation(front.u.ofdm.Constellation);
		transponder->terrestrial.transmission_mode = dvbApiToEtsiTransmitMode(front.u.ofdm.TransmissionMode);
		transponder->terrestrial.guard_interval = dvbApiToEtsiGuardInterval(front.u.ofdm.guardInterval);
		transponder->terrestrial.hierarchy_information = dvbApiToEtsiHierarchyInformation(front.u.ofdm.HierarchyInformation);
#else
		// FIXME
#endif
	}
}

void eFrontend::tune_all(eTransponder *trans) // called from within tune_qpsk, tune_qam, tune_ofdm
{
	tries=0;
	eSection::abortAll();
	lostlockcount=0;
	updateTransponderTimer.stop();
	transponder = trans;
	agctrimmode=agctriminit;
	berave=0;
}

int eFrontend::tune_qpsk(eTransponder *trans, 
		uint32_t Frequency, 		// absolute frequency in kHz
		int polarisation, 			// polarisation (polHor, polVert, ...)
		uint32_t SymbolRate,		// symbolrate in symbols/s (e.g. 27500000)
		uint8_t FEC_inner,			// FEC_inner (-1 for none, 0 for auto, but please don't use that)
		int Inversion,				// spectral inversion, INVERSION_OFF / _ON / _AUTO (but please...)
		eSatellite &sat)			// Satellite Data.. LNB, DiSEqC, switch..
{
	if (type != eSystemInfo::feSatellite)
		return -1;
	tune_all(trans);
	int finalTune=1;
//	eDebug("op = %d", trans->satellite.orbital_position );
	checkRotorLockTimer.stop();
	checkLockTimer.stop();

	if ( curRotorPos > 11000 )
		curRotorPos = 11000;

//	eDebug("ROTOR STOPPED 1");
	/* emit */ s_RotorStopped();

	if ( rotorTimer1.isActive() || rotorTimer2.isActive() )
	{
		eDebug("Switch ... but running rotor... send stop..");
		rotorTimer1.stop();
		rotorTimer2.stop();
		// ROTOR STOP
		sendDiSEqCCmd( 0x31, 0x60 );
		sendDiSEqCCmd( 0x31, 0x60 );
		lastRotorCmd=-1;
		curRotorPos=10000;
	}

	if (needreset > 1)
	{
#if HAVE_DVB_API_VERSION < 3
		ioctl(fd, FE_SET_POWER_STATE, FE_POWER_ON);
		usleep(150000);
#else
		curContTone = curVoltage = -1;
#endif
		InitDiSEqC();
		needreset=0;
	}

	transponder=trans;

	eSwitchParameter &swParams = sat.getSwitchParams();
	eLNB *lnb = sat.getLNB();
	// Variables to detect if DiSEqC must sent .. or not
	int csw = lnb->getDiSEqC().DiSEqCParam,
			ucsw = (lnb->getDiSEqC().uncommitted_cmd ?
				lnb->getDiSEqC().uncommitted_cmd : lastucsw),
			ToneBurst = (lnb->getDiSEqC().MiniDiSEqCParam ?
				lnb->getDiSEqC().MiniDiSEqCParam : lastToneBurst),
			RotorCmd = lastRotorCmd,
			sendRotorCmd=0;
//				SmatvFreq = -1;

	if ( !transponder->satellite.useable( lnb ) )
	{
//			eDebug("!!!!!!!!!!!!!!!! TUNED IN ETIMEDOUT 2 !!!!!!!!!!!!!!!!");
		/*emit*/ tunedIn(transponder, -ETIMEDOUT);
		return 0;
	}

	eSecCmdSequence seq;
#if HAVE_DVB_API_VERSION < 3
#define SEC_12V_OUT_OFF 0x10
#define SEC_12V_OUT_ON 0x11
	if ( eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020 &&
		ioctl(secfd, SEC_SET_VOLTAGE, lnb->get12VOut() ? SEC_12V_OUT_ON : SEC_12V_OUT_OFF) < 0 )
			eDebug("SEC_SET_VOLTAGE (12V Out) failed (%m)");
	secCommand *commands=0; // pointer to all sec commands
#else
	dvb_diseqc_master_cmd *commands=0;
#endif
    
	// num command counter
	int cmdCount=0;

	if (csw <= eDiSEqC::SENDNO)  // use AA/AB/BA/BB/SENDNO
	{
//			eDebug("csw=%d, csw<<2=%d", csw, csw << 2);
		if ( csw != eDiSEqC::SENDNO )
			csw = 0xF0 | ( csw << 2 );
		if ( polarisation==polHor)
		{
			csw |= 2;  // Horizontal
			eDebug("Horizontal");
		}
		else
			eDebug("Vertikal");

		if ( Frequency > lnb->getLOFThreshold() )
		{
			csw |= 1;   // 22 Khz enabled
			eDebug("Hi Band");
		}
		else
			eDebug("Low Band");
	}
	//else we sent directly the cmd 0xF0..0xFF

	if ( csw != eDiSEqC::SENDNO )
		eDebug("DiSEqC Switch cmd = %04x", csw);
	else
		eDebug("send no committed diseqc cmd !");

	// Rotor Support
	if ( lnb->getDiSEqC().DiSEqCMode == eDiSEqC::V1_2 && !noRotorCmd )
	{           
		bool useGotoXX=false;


		std::map<int,int,Orbital_Position_Compare>::iterator it =
			lnb->getDiSEqC().RotorTable.find( sat.getOrbitalPosition() );

		if (it != lnb->getDiSEqC().RotorTable.end())  // position for selected sat found ?
			RotorCmd=it->second;
		else  // entry not in table found
		{
			eDebug("Entry for %d,%d° not in Rotor Table found... i try gotoXX°", sat.getOrbitalPosition() / 10, sat.getOrbitalPosition() % 10 );

			useGotoXX=true;
			int pos = sat.getOrbitalPosition();
			int satDir = pos < 0 ? eDiSEqC::WEST : eDiSEqC::EAST;

			double SatLon = abs(pos)/10.00,
						 SiteLat = lnb->getDiSEqC().gotoXXLatitude,
						 SiteLon = lnb->getDiSEqC().gotoXXLongitude;

			if ( lnb->getDiSEqC().gotoXXLaDirection == eDiSEqC::SOUTH )
				SiteLat = -SiteLat;

			if ( lnb->getDiSEqC().gotoXXLoDirection == eDiSEqC::WEST )
				SiteLon = 360 - SiteLon;

			if (satDir == eDiSEqC::WEST )
				SatLon = 360 - SatLon;

			eDebug("siteLatitude = %lf, siteLongitude = %lf, %lf degrees", SiteLat, SiteLon, SatLon );
			double azimuth=calcAzimuth(SatLon, SiteLat, SiteLon );
			double elevation=calcElevation( SatLon, SiteLat, SiteLon );
			double declination=calcDeclination( SiteLat, azimuth, elevation );
			double satHourAngle=calcSatHourangle( azimuth, elevation, declination, SiteLat );
			eDebug("azimuth=%lf, elevation=%lf, declination=%lf, PolarmountHourAngle=%lf", azimuth, elevation, declination, satHourAngle );

			//
			// xphile: USALS fix for southern hemisphere
			//		
			if (SiteLat >= 0)
			{
				//
				// Northern Hemisphere
				//
				int tmp=(int)round( fabs( 180 - satHourAngle ) * 10.0 );
				RotorCmd = (tmp/10)*0x10 + gotoXTable[ tmp % 10 ];
	
				if (satHourAngle < 180)  // the east
					RotorCmd |= 0xE000;
				else                     // west
					RotorCmd |= 0xD000;
			}
			else
			{
				//
				// Southern Hemisphere
				//
				if (satHourAngle < 180)  // the east
				{
					int tmp=(int)round( fabs( satHourAngle ) * 10.0 );
					RotorCmd = (tmp/10)*0x10 + gotoXTable[ tmp % 10 ];
					RotorCmd |= 0xD000;
				}
				else
				{                     // west
					int tmp=(int)round( fabs( 360 - satHourAngle ) * 10.0 );
					RotorCmd = (tmp/10)*0x10 + gotoXTable[ tmp % 10 ];
					RotorCmd |= 0xE000;
				}
			}

			eDebug("RotorCmd = %04x", RotorCmd);
		}

		if ( RotorCmd != lastRotorCmd || (RotorCmd && curRotorPos == 10000) )  // rotorCmd must sent?
		{
			cmdCount=1; // this is the RotorCmd
			if ( ucsw != lastucsw )
				cmdCount++;
			if ( csw != lastcsw && csw & 0xF0) // NOT SENDNO
				cmdCount++;
			cmdCount += (cmdCount-1)*lnb->getDiSEqC().DiSEqCRepeats;

#if HAVE_DVB_API_VERSION < 3
			// allocate memory for all DiSEqC commands
			commands = new secCommand[cmdCount];
			commands[cmdCount-1].type = SEC_CMDTYPE_DISEQC_RAW;
			commands[cmdCount-1].u.diseqc.addr=0x31;     // normal positioner
			commands[cmdCount-1].u.diseqc.cmdtype=0xE0;  // no replay... first transmission
#else
			commands = new dvb_diseqc_master_cmd[cmdCount];
			commands[cmdCount-1].msg[0]=0xE0;
			commands[cmdCount-1].msg[1]=0x31;
#endif
			if ( useGotoXX )
			{
				eDebug("Rotor DiSEqC Param = %04x (useGotoXX)", RotorCmd);
#if HAVE_DVB_API_VERSION < 3
				commands[cmdCount-1].u.diseqc.cmd=0x6E; // gotoXX Drive Motor to Angular Position
				commands[cmdCount-1].u.diseqc.numParams=2;
				commands[cmdCount-1].u.diseqc.params[0]=((RotorCmd & 0xFF00) / 0x100);
				commands[cmdCount-1].u.diseqc.params[1]=RotorCmd & 0xFF;
#else
				commands[cmdCount-1].msg[2]=0x6E;
				commands[cmdCount-1].msg[3]=((RotorCmd & 0xFF00) / 0x100);
				commands[cmdCount-1].msg[4]=RotorCmd & 0xFF;
				commands[cmdCount-1].msg_len=5;
#endif
			}
			else
			{
				eDebug("Rotor DiSEqC Param = %02x (use stored position)", RotorCmd);
#if HAVE_DVB_API_VERSION < 3
				commands[cmdCount-1].u.diseqc.cmd=0x6B;  // goto stored sat position
				commands[cmdCount-1].u.diseqc.numParams=1;
				commands[cmdCount-1].u.diseqc.params[0]=RotorCmd;
#else
				commands[cmdCount-1].msg[2]=0x6B;
				commands[cmdCount-1].msg[3]=RotorCmd;
				commands[cmdCount-1].msg_len=4;
#endif
			}
			sendRotorCmd++;
		}
	}

	if ( curRotorPos > 10000 && !sendRotorCmd )
	{
		eDebug("reset Rotor cmd");
		curRotorPos = 10000;
	}

#if 0
	else if ( lnb->getDiSEqC().DiSEqCMode == eDiSEqC::SMATV )
	{
		SmatvFreq=Frequency;
		if ( lastSmatvFreq != SmatvFreq )
		{
			if ( lnb->getDiSEqC().uncommitted_cmd && lastucsw != ucsw)
				cmdCount=3;
			else
				cmdCount=2;
			cmdCount += (cmdCount-1)*lnb->getDiSEqC().DiSEqCRepeats;

			// allocate memory for all DiSEqC commands
			commands = new secCommand[cmdCount];

			commands[cmdCount-1].type = SEC_CMDTYPE_DISEQC_RAW;
			commands[cmdCount-1].u.diseqc.addr = 0x71;	// intelligent slave interface for multi-master bus
			commands[cmdCount-1].u.diseqc.cmd = 0x58;	  // write channel frequency
			commands[cmdCount-1].u.diseqc.cmdtype = 0xE0;
			commands[cmdCount-1].u.diseqc.numParams = 3;
			commands[cmdCount-1].u.diseqc.params[0] = (((Frequency / 10000000) << 4) & 0xF0) | ((Frequency / 1000000) & 0x0F);
			commands[cmdCount-1].u.diseqc.params[1] = (((Frequency / 100000) << 4) & 0xF0) | ((Frequency / 10000) & 0x0F);
			commands[cmdCount-1].u.diseqc.params[2] = (((Frequency / 1000) << 4) & 0xF0) | ((Frequency / 100) & 0x0F);
		}
	}
#endif
	if ( lnb->getDiSEqC().DiSEqCMode >= eDiSEqC::V1_0 )
	{
//			eDebug("ucsw=%d lastucsw=%d csw=%d lastcsw=%d", ucsw, lastucsw, csw, lastcsw);
		if ( ucsw != lastucsw || csw != lastcsw || (ToneBurst && ToneBurst != lastToneBurst) )
		{
//			eDebug("cmdCount=%d", cmdCount);
			int loops;
			if ( cmdCount )  // Smatv or Rotor is avail...
				loops = cmdCount - 1;  // do not overwrite rotor cmd
			else // no rotor or smatv
			{
				// DiSEqC Repeats and uncommitted switches only when DiSEqC >= V1_1
				if ( lnb->getDiSEqC().DiSEqCMode >= eDiSEqC::V1_1	&& ucsw != lastucsw )
					cmdCount++;
				if ( csw != lastcsw && csw & 0xF0 )
					cmdCount++;
				cmdCount += cmdCount*lnb->getDiSEqC().DiSEqCRepeats;
				loops = cmdCount;

				if ( cmdCount )
#if HAVE_DVB_API_VERSION < 3
					commands = new secCommand[cmdCount];
#else
					commands = new dvb_diseqc_master_cmd[cmdCount];
#endif
			}

			for ( int i = 0; i < loops;)  // fill commands...
			{
				enum { UNCOMMITTED, COMMITTED } cmdbefore = COMMITTED;
#if HAVE_DVB_API_VERSION < 3
				commands[i].type = SEC_CMDTYPE_DISEQC_RAW;
				commands[i].u.diseqc.cmdtype= i ? 0xE1 : 0xE0; // repeated or not repeated transm.
				commands[i].u.diseqc.numParams=1;
				commands[i].u.diseqc.addr=0x10;
#else
				commands[i].msg[0]= i ? 0xE1 : 0xE0;
				commands[i].msg[1]=0x10;
				commands[i].msg_len=4;
#endif
				if ( ( lnb->getDiSEqC().SwapCmds
					&& lnb->getDiSEqC().DiSEqCMode > eDiSEqC::V1_0
					&& ucsw != lastucsw )
					|| !(csw & 0xF0) )
				{
					cmdbefore = UNCOMMITTED;
#if HAVE_DVB_API_VERSION < 3
					commands[i].u.diseqc.params[0] = ucsw;
					commands[i].u.diseqc.cmd=0x39;
#else
					commands[i].msg[2]=0x39;
					commands[i].msg[3]=ucsw;
#endif
				}
				else
				{
#if HAVE_DVB_API_VERSION < 3
					commands[i].u.diseqc.params[0] = csw;
					commands[i].u.diseqc.cmd=0x38;
#else
					commands[i].msg[2]=0x38;
					commands[i].msg[3]=csw;
#endif
				}
//				eDebug("normalCmd");
//				eDebug("0x%02x,0x%02x,0x%02x,0x%02x", commands[i].u.diseqc.cmdtype, commands[i].u.diseqc.addr, commands[i].u.diseqc.cmd, commands[i].u.diseqc.params[0]);
				i++;
				if ( i < loops
					&& ( ( (cmdbefore == COMMITTED) && ucsw )
						|| ( (cmdbefore == UNCOMMITTED) && csw & 0xF0 ) ) )
				{
					memcpy( &commands[i], &commands[i-1], sizeof(commands[i]) );
					if ( cmdbefore == COMMITTED )
					{
#if HAVE_DVB_API_VERSION < 3
						commands[i].u.diseqc.cmd=0x39;
						commands[i].u.diseqc.params[0]=ucsw;
#else
						commands[i].msg[2]=0x39;
						commands[i].msg[3]=ucsw;
#endif
					}
					else
					{
#if HAVE_DVB_API_VERSION < 3
						commands[i].u.diseqc.cmd=0x38;
						commands[i].u.diseqc.params[0]=csw;
#else
						commands[i].msg[2]=0x38;
						commands[i].msg[3]=csw;
#endif
					}
//						eDebug("0x%02x,0x%02x,0x%02x,0x%02x", commands[i].u.diseqc.cmdtype, commands[i].u.diseqc.addr, commands[i].u.diseqc.cmd, commands[i].u.diseqc.params[0]);
					i++;
				}
			}
		}
	}
	if ( !cmdCount)
	{
		eDebug("send no DiSEqC");
		seq.commands=0;
	}

	seq.numCommands=cmdCount;
	eDebug("%d DiSEqC cmds to send", cmdCount);

	seq.toneBurst = eSecCmdSequence::NONE;
	if ( ucsw != lastucsw || csw != lastcsw || (ToneBurst && ToneBurst != lastToneBurst) )
	{
		if ( lnb->getDiSEqC().MiniDiSEqCParam == eDiSEqC::A )
		{
			eDebug("Toneburst A");
			seq.toneBurst=eSecCmdSequence::TONEBURST_A;
		}
		else if ( lnb->getDiSEqC().MiniDiSEqCParam == eDiSEqC::B )
		{
			eDebug("Toneburst B");
			seq.toneBurst=eSecCmdSequence::TONEBURST_B;
		}
		else
			eDebug("no Toneburst (MiniDiSEqC)");
	}
     
	// no DiSEqC related Stuff

	// calc Frequency
	int local = Frequency - ( Frequency > lnb->getLOFThreshold() ? lnb->getLOFHi() : lnb->getLOFLo() );
#if HAVE_DVB_API_VERSION < 3
	front.Frequency = abs(local);
#else
	front.frequency = abs(local);
#endif

	// set Continuous Tone ( 22 Khz... low - high band )
	if ( (swParams.HiLoSignal == eSwitchParameter::ON) || ( (swParams.HiLoSignal == eSwitchParameter::HILO) && (Frequency > lnb->getLOFThreshold()) ) )
		seq.continuousTone = eSecCmdSequence::TONE_ON;
	else if ( (swParams.HiLoSignal == eSwitchParameter::OFF) || ( (swParams.HiLoSignal == eSwitchParameter::HILO) && (Frequency <= lnb->getLOFThreshold()) ) )
		seq.continuousTone = eSecCmdSequence::TONE_OFF;

	seq.increasedVoltage = lnb->getIncreasedVoltage();

	// Voltage( 0/14/18V  vertical/horizontal )
	if ( swParams.VoltageMode == eSwitchParameter::_14V || ( polarisation == polVert && swParams.VoltageMode == eSwitchParameter::HV )  )
		voltage = eSecCmdSequence::VOLTAGE_13;
	else if ( swParams.VoltageMode == eSwitchParameter::_18V || ( polarisation==polHor && swParams.VoltageMode == eSwitchParameter::HV)  )
		voltage = eSecCmdSequence::VOLTAGE_18;
	else
		voltage = eSecCmdSequence::VOLTAGE_OFF;
     
	// set cmd ptr in sequence..
	seq.commands=commands;

	// handle DiSEqC Rotor
	if ( sendRotorCmd )
	{
#if 0
		eDebug("handle DISEqC Rotor.. cmdCount = %d", cmdCount);
		eDebug("0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
			commands[cmdCount-1].u.diseqc.cmdtype,
			commands[cmdCount-1].u.diseqc.addr,
			commands[cmdCount-1].u.diseqc.cmd,
			commands[cmdCount-1].u.diseqc.params[0],
			commands[cmdCount-1].u.diseqc.params[1]);
#endif

		// drive rotor with 18V when we can measure inputpower
		// or we know which orbital pos currently is selected
		if ( lnb->getDiSEqC().useRotorInPower&1 )
		{
			if ( eSystemInfo::getInstance()->canMeasureLNBCurrent() == 1 )  // can measure with voltage > 13V ?
				seq.voltage = eSecCmdSequence::VOLTAGE_18;
			else
				seq.voltage = eSecCmdSequence::VOLTAGE_13;
		}
		else
			seq.voltage = voltage;

		lastRotorCmd=RotorCmd;

		increased = seq.increasedVoltage;
		newPos = sat.getOrbitalPosition();

		if ( lnb->getDiSEqC().useRotorInPower&1 )
		{
			curRotorPos=5000;
			DeltaA=(lnb->getDiSEqC().useRotorInPower & 0x0000FF00) >> 8;
			RotorUseInputPower(seq, lnb );
			finalTune=0;
		}
		else
		{
			if ( !eDVB::getInstance()->getScanAPI() )
			{
				eDebug("ROTOR RUNNING EMIT");
				/* emit */ s_RotorRunning( newPos );
			}
			RotorUseTimeout(seq, lnb );
			curRotorPos=11000;  // Rotor cmd sent
		}
	}
	else if ( lastucsw != ucsw || ( ToneBurst && lastToneBurst != ToneBurst) )
	{
send:
		seq.voltage=voltage;
		if ( SendSequence(seq) < 0 )
		{
			eDebug("SendSequence failed (%m)");
			return -1;
		}
		else if ( lnb->getDiSEqC().DiSEqCMode >= eDiSEqC::V1_1 && lnb->getDiSEqC().SeqRepeat )  // Sequence Repeat ?
		{
			usleep( 100000 ); // between seq repeats we wait 80ms
			SendSequence(seq);  // just do it *g*
		}
	}
	else if ( lastcsw != csw )
	{
		// faster zap workaround... send only diseqc when satpos changed
		if ( lnb->getDiSEqC().FastDiSEqC && csw && (csw / 4) == (lastcsw / 4) )
		{
			eDebug("Satellite has not changed.. don't send DiSEqC cmd (Fast DiSEqC)");
			seq.numCommands=0;
		}
		goto send; // jump above...
	}
	else
		eDebug("no Band or Polarisation changed .. don't send Sequence");

	lastcsw = csw;
	lastucsw = ucsw;
	lastToneBurst = ToneBurst;
	lastLNB = lnb;  /* important.. for the correct timeout
										 between normal diseqc cmd and rotor cmd */

	// delete allocated memory
	delete [] commands;

// fill in Frontend data
#if HAVE_DVB_API_VERSION < 3
	front.Inversion=(Inversion == 2 ? INVERSION_AUTO :
		(Inversion?INVERSION_ON:INVERSION_OFF) );
	front.u.qpsk.FEC_inner=etsiToDvbApiFEC(FEC_inner);
	front.u.qpsk.SymbolRate=SymbolRate;
#else
	front.inversion=(Inversion == 2 ? INVERSION_AUTO :
		(Inversion?INVERSION_ON:INVERSION_OFF) );
	front.u.qpsk.fec_inner=etsiToDvbApiFEC(FEC_inner);
	front.u.qpsk.symbol_rate=SymbolRate;
#endif

	if (finalTune)
		return setFrontend();

	return 0;
}

int eFrontend::tune_qam(eTransponder *trans, 
		uint32_t Frequency,			// absolute frequency in kHz
		uint32_t SymbolRate,		// symbolrate in symbols/s (e.g. 6900000)
		uint8_t FEC_inner,			// FEC_inner (-1 for none, 0 for auto, but please don't use that). normally -1.
		int Inversion,					// spectral inversion, INVERSION_OFF / _ON / _AUTO (but please...)
		int QAM)								// Modulation, QAM_xx
{
	eDebug("Cable Frontend detected");
	tune_all(trans);

	if (needreset > 1)
	{
#if HAVE_DVB_API_VERSION < 3
		ioctl(fd, FE_SET_POWER_STATE, FE_POWER_ON);
		usleep(150000);
#endif
		needreset=0;
	}

#if HAVE_DVB_API_VERSION < 3
	front.Inversion=(Inversion == 2 ? INVERSION_AUTO :
		(Inversion?INVERSION_ON:INVERSION_OFF) );
	front.Frequency=Frequency;
	front.u.qam.QAM=etsiToDvbApiModulation(QAM);
	front.u.qam.FEC_inner=etsiToDvbApiFEC(FEC_inner);
	front.u.qam.SymbolRate=SymbolRate;
#else
	front.inversion=(Inversion == 2 ? INVERSION_AUTO :
		(Inversion?INVERSION_ON:INVERSION_OFF) );
	front.frequency = Frequency*1000; // dbox2 v3 drivers need frequency as Hz
	front.u.qam.modulation=etsiToDvbApiModulation(QAM);
	front.u.qam.fec_inner=etsiToDvbApiFEC(FEC_inner);
	front.u.qam.symbol_rate=SymbolRate;
#endif
	return setFrontend();
}

int eFrontend::tune_ofdm(eTransponder *trans,
		int centre_frequency,
		int bandwidth,
		int constellation,
		int hierarchy_information,
		int code_rate_hp,
		int code_rate_lp,
		int guard_interval,
		int transmission_mode,
		int inversion)
{
	eDebug("DVB-T Frontend detected");
	tune_all(trans);

	if (needreset > 1)
	{
#if HAVE_DVB_API_VERSION < 3
		ioctl(fd, FE_SET_POWER_STATE, FE_POWER_ON);
		usleep(150000);
#endif
		needreset=0;
	}

#if HAVE_DVB_API_VERSION < 3
	front.Inversion=(inversion == 2 ? INVERSION_AUTO :
		(inversion?INVERSION_ON:INVERSION_OFF) );
	front.Frequency = centre_frequency;
	front.u.ofdm.bandWidth=etsiToDvbApiBandwidth(bandwidth);
	front.u.ofdm.HP_CodeRate=etsiToDvbApiCodeRate(code_rate_hp);
	front.u.ofdm.LP_CodeRate=etsiToDvbApiCodeRate(code_rate_lp);
	front.u.ofdm.Constellation=etsiToDvbApiConstellation(constellation);
	front.u.ofdm.TransmissionMode=etsiToDvbApiTransmitMode(transmission_mode);
	front.u.ofdm.guardInterval=etsiToDvbApiGuardInterval(guard_interval);
	front.u.ofdm.HierarchyInformation=etsiToDvbApiHierarchyInformation(hierarchy_information);
#else
	front.inversion=(inversion == 2 ? INVERSION_AUTO :
		(inversion?INVERSION_ON:INVERSION_OFF) );
	front.frequency = centre_frequency;
	front.u.ofdm.bandwidth=etsiToDvbApiBandwidth(bandwidth);
	front.u.ofdm.code_rate_HP=etsiToDvbApiCodeRate(code_rate_hp);
	front.u.ofdm.code_rate_LP=etsiToDvbApiCodeRate(code_rate_lp);
	front.u.ofdm.constellation=etsiToDvbApiConstellation(constellation);
	front.u.ofdm.transmission_mode=etsiToDvbApiTransmitMode(transmission_mode);
	front.u.ofdm.guard_interval=etsiToDvbApiGuardInterval(guard_interval);
	front.u.ofdm.hierarchy_information=etsiToDvbApiHierarchyInformation(hierarchy_information);
#endif
	return setFrontend();
}

int eFrontend::savePower()
{
	if (type == eSystemInfo::feUnknown)
		return -1;
	transponder=0;
	checkLockTimer.stop();
	checkRotorLockTimer.stop();
	updateTransponderTimer.stop();
	rotorTimer1.stop();
	rotorTimer2.stop();
#if HAVE_DVB_API_VERSION < 3
	if (secfd != -1)
	{        
		eSecCmdSequence seq;
		seq.commands=0;
		seq.numCommands=0;
		seq.voltage= eSecCmdSequence::VOLTAGE_OFF;
		seq.continuousTone = eSecCmdSequence::TONE_OFF;
		seq.toneBurst = eSecCmdSequence::NONE;
		if (SendSequence(seq) < 0 )
		{
			eDebug("SendSequence failed (%m)");
			return -1;
		}
	}
	if ( ioctl(fd, FE_SET_POWER_STATE, FE_POWER_OFF) < 0 )
		eDebug("FE_SET_POWER_STATE failed (%m)");
#else
	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF) < 0)
		eDebug("FE_SET_VOLTAGE (off) failed (%m)");
	if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) < 0)
		eDebug("FE_SET_TONE (off) failed (%m)");
#endif
	transponder=0;
	needreset = 2;
	return 0;
}

void eFrontend::setTerrestrialAntennaVoltage(bool state)
{
	if (type != eSystemInfo::feTerrestrial)
		return;
#if HAVE_DVB_API_VERSION < 3
	int secfd=::open(SEC_DEV, O_RDWR);
	if (secfd<0)
	{
		eDebug("open secfd failed(%m)");
		return;
	}
	if ( ::ioctl(secfd, SEC_SET_VOLTAGE, state ? SEC_VOLTAGE_OFF : SEC_VOLTAGE_13) < 0 )
		eDebug("SEC_SET_VOLTAGE (-T) failed (%m)");
	::close(secfd);
#endif
}
