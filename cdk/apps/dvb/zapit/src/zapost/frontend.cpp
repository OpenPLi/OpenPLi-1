/*
 * $Id: frontend.cpp,v 1.29.2.1 2003/07/06 08:44:53 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* system c */
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* system c++ */
#include <iostream>

/* zapit */
#include <zapit/frontend.h>
#include <zapit/getservices.h>
#include <zapit/nit.h>
#include <zapit/sdt.h>
#include <zapit/settings.h>

extern std::map <uint32_t, transponder> transponders;

/* constructor */
CFrontend::CFrontend ()
{
	failed = false;
	tuned = false;
	currentFrequency = 0;
	currentTsidOnid = 0;
	diseqcRepeats = 0;
	diseqcType = NO_DISEQC;

	info = new FrontendInfo();

	if ((frontend_fd = open(FRONTEND_DEVICE, O_RDWR|O_NONBLOCK)) < 0)
	{
		perror(FRONTEND_DEVICE);
		initialized = false;
	}
	else if (ioctl(frontend_fd, FE_GET_INFO, info) < 0)
	{
		perror("FE_GET_INFO");
		initialized = false;
	}
	else if ((sec_fd = open(SEC_DEVICE, O_RDWR)) < 0)
	{
		perror(SEC_DEVICE);
		initialized = false;
	}
	else
	{
		secSetTone(SEC_TONE_OFF);
		secSetVoltage(SEC_VOLTAGE_13);
		initialized = true;
	}
}

/* destructor */
CFrontend::~CFrontend ()
{
	delete info;
	close(sec_fd);
	close(frontend_fd);
}

/*
 * ost frontend api
 */

CodeRate CFrontend::getFEC (uint8_t FEC_inner)
{
	switch (FEC_inner)
	{
	case 0x01:
		return FEC_1_2;
	case 0x02:
		return FEC_2_3;
	case 0x03:
		return FEC_3_4;
	case 0x04:
		return FEC_5_6;
	case 0x05:
		return FEC_7_8;
	case 0x0F:
		return FEC_NONE;
	default:
		return FEC_AUTO;
	}
}

Modulation CFrontend::getModulation (uint8_t modulation)
{
	switch (modulation)
	{
	case 0x01:
		return QAM_16;
	case 0x02:
		return QAM_32;
	case 0x03:
		return QAM_64;
	case 0x04:
		return QAM_128;
	case 0x05:
		return QAM_256;
	default:
		return QAM_64;
	}
}

unsigned int CFrontend::getFrequency ()
{
	if (info->type == FE_QPSK)
	{
		if (currentToneMode == SEC_TONE_OFF)
		{
			return currentFrequency + lnbOffsetsLow[currentDiseqc];
		}
		else
		{
			return currentFrequency + lnbOffsetsHigh[currentDiseqc];
		}
	}
	else
	{
		return currentFrequency;
	}
}

unsigned char CFrontend::getPolarization ()
{
	if (currentVoltage == SEC_VOLTAGE_13)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CFrontend::selfTest ()
{
	if (ioctl(frontend_fd, FE_SELFTEST) < 0)
	{
		perror("FE_SELFTEST");
		failed = true;
	}
	else
	{
		failed = false;
	}
}

void CFrontend::setPowerState (FrontendPowerState state)
{
	if (ioctl(frontend_fd, FE_SET_POWER_STATE, state) < 0)
	{
		perror("FE_SET_POWER_STATE");
		failed = true;
	}
	else
	{
		failed = false;
	}
}

const FrontendPowerState CFrontend::getPowerState ()
{
	FrontendPowerState state;

	if (ioctl(frontend_fd, FE_GET_POWER_STATE, &state) < 0)
	{
		perror("FE_GET_POWER_STATE");
		failed = true;
	}

	return state;
}

const FrontendStatus CFrontend::getStatus ()
{
	FrontendStatus status;

	if (ioctl(frontend_fd, FE_READ_STATUS, &status) < 0)
	{
		perror("FE_READ_STATUS");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return status;
}

const uint32_t CFrontend::getBitErrorRate ()
{
	uint32_t ber;

	if (ioctl(frontend_fd, FE_READ_BER, &ber) < 0)
	{
		perror("FE_READ_BER");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return ber;
}

const int32_t CFrontend::getSignalStrength ()
{
	int32_t strength;

	if (ioctl(frontend_fd, FE_READ_SIGNAL_STRENGTH, &strength) < 0)
	{
		perror("FE_READ_SIGNAL_STRENGTH");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return strength;
}

const int32_t CFrontend::getSignalNoiseRatio ()
{
	int32_t snr;

	if (ioctl(frontend_fd, FE_READ_SNR, &snr) < 0)
	{
		perror("FE_READ_SNR");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return snr;
}

const uint32_t CFrontend::getUncorrectedBlocks ()
{
	uint32_t blocks;

	if (ioctl(frontend_fd, FE_READ_UNCORRECTED_BLOCKS, &blocks) < 0)
	{
		perror("FE_READ_UNCORRECTED_BLOCKS");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return blocks;
}

const uint32_t CFrontend::getNextFrequency (uint32_t frequency)
{
	if (ioctl(frontend_fd, FE_GET_NEXT_FREQUENCY, &frequency) < 0)
	{
		perror("FE_GET_NEXT_FREQUENCY");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return frequency;
}

const uint32_t CFrontend::getNextSymbolRate (uint32_t rate)
{
	if (ioctl(frontend_fd, FE_GET_NEXT_SYMBOL_RATE, rate) < 0)
	{
		perror("FE_GET_NEXT_SYMBOL_RATE");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return rate;
}

void CFrontend::setFrontend (FrontendParameters *feparams)
{
	FrontendEvent *event = new FrontendEvent();

	std::cout << "[CFrontend::setFrontend] freq " << feparams->Frequency << std::endl;

	/* clear old events */
	while (ioctl(frontend_fd, FE_GET_EVENT, event) >= 0)
		std::cout << "[CFrontend::setFrontend] discard event" << std::endl;

	delete event;

	if (ioctl(frontend_fd, FE_SET_FRONTEND, feparams) < 0)
	{
		perror("FE_SET_FRONTEND");
		failed = true;
	}
	else
	{
		failed = false;
	}
}

const FrontendParameters *CFrontend::getFrontend ()
{
	FrontendParameters *feparams = new FrontendParameters();

	if (ioctl(frontend_fd, FE_GET_FRONTEND, feparams) < 0)
	{
		perror("FE_GET_FRONTEND");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return feparams;
}

const bool CFrontend::getEvent ()
{
	FrontendEvent *event = new FrontendEvent();

	struct pollfd pfd[1];

	pfd[0].fd = frontend_fd;
	pfd[0].events = POLLIN | POLLPRI;

	failed = true;

	switch (poll(pfd, 1, 10000))
	{
	case -1:
		perror("[CFrontend::getEvent] poll");
		break;

	case 0:
		std::cerr << "[CFrontend::getEvent] timeout" << std::endl;
		break;

	default:
		if (pfd[0].revents & POLLIN)
		{
			if (ioctl(frontend_fd, FE_GET_EVENT, event) < 0)
			{
				perror("[CFrontend::getEvent] FE_GET_EVENT");
				break;
			}
			else
			{
				failed = false;
			}

			switch (event->type)
			{
			case FE_UNEXPECTED_EV:
				std::cerr << "[CFrontend::getEvent] FE_UNEXPECTED_EV" << std::endl;
				break;

			case FE_FAILURE_EV:
				std::cerr << "[CFrontend::getEvent] FE_FAILURE_EV" << std::endl;
				break;

			case FE_COMPLETION_EV:
				currentFrequency = event->u.completionEvent.Frequency;
				std::cout << "[CFrontend::getEvent] FE_COMPLETION_EV: freq " << currentFrequency << std::endl;
				tuned = true;
				break;
			}
		}
		else
		{
			std::cerr << "[CFrontend::getEvent] pfd[0].revents: " << pfd[0].revents << std::endl;
		}
		break;
	}

	if (tuned == false)
	{
		currentFrequency = 0;
		currentTsidOnid = 0;
	}

	delete event;
	return tuned;
}

/*
 * ost sec api
 */
void CFrontend::secSetTone (secToneMode toneMode)
{
	if (ioctl(sec_fd, SEC_SET_TONE, toneMode) < 0)
	{
		perror("SEC_SET_TONE");
		failed = true;
	}
	else
	{
		currentToneMode = toneMode;
		failed = false;
	}
}

void CFrontend::secSetVoltage (secVoltage voltage)
{
	if (ioctl(sec_fd, SEC_SET_VOLTAGE, voltage) < 0)
	{
		perror("SEC_SET_VOLTAGE");
		failed = true;
	}
	else
	{
		currentVoltage = voltage;
		failed = false;
	}
}

const secStatus *CFrontend::secGetStatus ()
{
	secStatus *status = new secStatus();

	if (ioctl(sec_fd, SEC_GET_STATUS, status) < 0)
	{
		perror("SEC_GET_STATUS");
		failed = true;
	}
	else
	{
		failed = false;
	}

	return status;
}

void CFrontend::secSendSequence (secCmdSequence *sequence)
{
	if (ioctl(sec_fd, SEC_SEND_SEQUENCE, sequence) < 0)
	{
		perror("SEC_SEND_SEQUENCE");
		failed = true;
	}
	else
	{
		currentToneMode = sequence->continuousTone;
		currentVoltage = sequence->voltage;
		failed = false;
	}
}

#if 0
void CFrontend::secResetOverload ()
{
	if (ioctl(sec_fd, SEC_RESET_OVERLOAD) < 0)
	{
		perror("SEC_RESET_OVERLOAD");
		failed = true;
	}
	else
	{
		failed = false;
	}
}
#endif

/*
 * zapit frontend api
 */
const bool CFrontend::tuneChannel (CZapitChannel *channel)
{
	bool noNit = false;

	if (transponders.find(channel->getTsidOnid()) == transponders.end())
	{
		/* if not found, look up in nit */
		if ((parse_nit(channel->getDiSEqC()) < 0))
		{
			currentTsidOnid = get_sdt_TsidOnid();
			noNit = true;
		}
		else if (transponders.find(channel->getTsidOnid()) == transponders.end())
		{
			return false;
		}
	}

	currentTsidOnid = noNit ? currentTsidOnid : channel->getTsidOnid();

	std::map <uint32_t, transponder>::iterator transponder = transponders.find(currentTsidOnid);

	return tuneFrequency
	(
		transponder->second.feparams,
		transponder->second.polarization,
		transponder->second.DiSEqC
	);
}

const bool CFrontend::tuneFrequency (FrontendParameters feparams, uint8_t polarization, uint8_t diseqc)
{
	bool secChanged = false;
	tuned = false;

	/* sec */
	if (info->type == FE_QPSK)
	{
		secToneMode toneMode;
		secVoltage voltage;

		/* tone */
		if (feparams.Frequency < 11700000)
		{
			/* low band */
			feparams.Frequency -= lnbOffsetsLow[diseqc];
			toneMode = SEC_TONE_OFF;
		}
		else
		{
			/* high band */
			feparams.Frequency -= lnbOffsetsHigh[diseqc];
			toneMode = SEC_TONE_ON;
		}

		/* voltage */
		if (polarization == 1)
		{
			/* vertical */
			voltage = SEC_VOLTAGE_13;
		}
		else
		{
			/* horizontal */
			voltage = SEC_VOLTAGE_18;
		}

		/* do diseqc stuff */
		switch (diseqcType)
		{
		case NO_DISEQC:
			if ((currentToneMode != toneMode) || (currentVoltage != voltage))
			{
				if (currentToneMode != SEC_TONE_OFF)
					secSetTone(SEC_TONE_OFF);
				if (currentVoltage != voltage)
					secSetVoltage(voltage);
				if (toneMode != SEC_TONE_OFF)
					secSetTone(SEC_TONE_ON);
				secChanged = true;
			}
			break;

		case MINI_DISEQC:
			if ((currentVoltage != voltage) || (currentToneMode != toneMode) || (currentDiseqc != diseqc))
			{
				sendDiseqcMiniCommand(toneMode, voltage, diseqc);
				secChanged = true;
			}
			break;

		case DISEQC_1_0:
			if ((currentVoltage != voltage) || (currentToneMode != toneMode) || (currentDiseqc != diseqc))
			{
				sendDiseqcCommand(toneMode, voltage, diseqc, 0);
				secChanged = true;
			}
			break;

		case DISEQC_1_1:
			if ((currentVoltage != voltage) || (currentToneMode != toneMode) || (currentDiseqc != diseqc))
			{
				sendDiseqcCommand(toneMode, voltage, diseqc, diseqcRepeats);
				secChanged = true;
			}
			break;

		case SMATV_REMOTE_TUNING:
			if ((currentFrequency != feparams.Frequency) || (currentVoltage != voltage) || (currentToneMode != toneMode) || (currentDiseqc != diseqc))
			{
				sendDiseqcSmatvRemoteTuningCommand(toneMode, voltage, diseqc, feparams.Frequency);
				secChanged = true;
			}
			break;
		}
	}

	if ((currentFrequency != feparams.Frequency) || (secChanged == true))
	{
		/* tune */
		setFrontend(&feparams);

		/* wait for completion */
		getEvent();

		/*
		 * the frontend got lock at a different frequency
		 * than requested, so we need to look up the tsid/onid.
		 */
		if ((tuned == true) && (currentFrequency != feparams.Frequency))
		{
			currentTsidOnid = get_sdt_TsidOnid();
		}
	}

	return tuned;
}

/*
 * zapit diseqc api
 */
const bool CFrontend::sendDiseqcMiniCommand (secToneMode toneMode, secVoltage voltage, uint8_t diseqc)
{
	secCmdSequence *sequence = new secCmdSequence();

	switch (diseqc)
	{
	case 0:
		sequence->miniCommand = SEC_MINI_A;
		break;
	case 1:
		sequence->miniCommand = SEC_MINI_B;
		break;
	default:
		failed = true;
		return false;
	}

	sequence->numCommands = 0;
	sequence->continuousTone = toneMode;
	sequence->voltage = voltage;
	sequence->commands = NULL;

	secSendSequence(sequence);

	delete sequence;

	if (failed == false)
	{
		currentDiseqc = diseqc;
		return true;
	}
	else
	{
		return false;
	}
}

const bool CFrontend::sendDiseqcPowerOn ()
{
	return sendDiseqcZeroByteCommand(0x10, 0x03);
}

const bool CFrontend::sendDiseqcReset ()
{
	return sendDiseqcZeroByteCommand(0x10, 0x00);
}

const bool CFrontend::sendDiseqcStandby ()
{
	return sendDiseqcZeroByteCommand(0x10, 0x02);
}

const bool CFrontend::sendDiseqcZeroByteCommand (uint8_t addr, uint8_t cmd)
{
	secCmdSequence *sequence = new secCmdSequence();
	sequence->commands = new secCommand();

	sequence->miniCommand = SEC_MINI_NONE;
	sequence->continuousTone = SEC_TONE_OFF;
	sequence->voltage = SEC_VOLTAGE_OFF;

	sequence->commands[0].type = SEC_CMDTYPE_DISEQC;
	sequence->commands[0].u.diseqc.addr = addr;
	sequence->commands[0].u.diseqc.cmd = cmd;
	sequence->commands[0].u.diseqc.numParams = 0;

	secSendSequence(sequence);

	delete sequence->commands;
	delete sequence;

	if (failed == false)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * this can handle a maximum of 12 LNBs,
 * connected to 3 cascaded switches.
 * for up to 64 LNBs, a more complex
 * setup will be needed.
 */
const bool CFrontend::sendDiseqcCommand (secToneMode toneMode, secVoltage voltage, uint8_t diseqc, uint32_t repeats)
{
	/* repeats = number of cascaded switches minus one */
	if (repeats > 2)
	{
		std::cerr << "[CFrontend::sendDiseqcCommand] max. 2 repeats allowed." << std::endl;
		return false;
	}

	/* diseqc = currently selected lnb */
	if (diseqc > 11)
	{
		std::cerr << "[CFrontend::sendDiseqcCommand] currently only 12 DiSEqC positions are supported." << std::endl;
		return false;
	}

	if (diseqc >= (repeats + 1) << 2)
	{
		std::cerr << "[CFrontend::sendDiseqcCommand] not enough repeats (" << repeats << ") for requested DiSEqC position (" << diseqc << ")." << std::endl;
		return false;
	}

	secCmdSequence *sequence = new secCmdSequence();
	sequence->commands = new secCommand[(repeats * 2) + 1];

	sequence->miniCommand = SEC_MINI_NONE;
	sequence->continuousTone = toneMode;
	sequence->voltage = voltage;

#ifdef DBOX2
	sequence->commands[0].type = SEC_CMDTYPE_DISEQC_RAW;
	sequence->commands[0].u.diseqc.cmdtype = 0xE0;	/* from master, no reply, 1st transmission */
#else
	sequence->commands[0].type = SEC_CMDTYPE_DISEQC;
#endif
	sequence->commands[0].u.diseqc.addr = 0x10;	/* any lnb switcher or smatv */
	sequence->commands[0].u.diseqc.cmd = 0x38;	/* write to port group 0 (committed switches) */

	if (diseqc < 4)
	{
		sequence->commands[0].u.diseqc.numParams = 1;
		sequence->commands[0].u.diseqc.params[0] = 0xF0 | ((diseqc << 2) & 0x0F) | (toneMode == SEC_TONE_ON ? 1 : 0) | (voltage == SEC_VOLTAGE_18 ? 2 : 0);
	}
	else
	{
		sequence->commands[0].u.diseqc.numParams = 0;
	}

	for (sequence->numCommands = 1; sequence->numCommands < (repeats << 1) + 1; sequence->numCommands += 2)
	{
#ifdef DBOX2
		sequence->commands[sequence->numCommands].type = SEC_CMDTYPE_DISEQC_RAW;
		sequence->commands[sequence->numCommands].u.diseqc.cmdtype = (sequence->numCommands - 1 ? 0xE1 : 0xE0);
		sequence->commands[sequence->numCommands + 1].type = SEC_CMDTYPE_DISEQC_RAW;
		sequence->commands[sequence->numCommands + 1].u.diseqc.cmdtype = 0xE1;	/* from master, no reply, repeated transmission */
#else
		sequence->commands[sequence->numCommands].type = SEC_CMDTYPE_DISEQC;
		sequence->commands[sequence->numCommands + 1].type = SEC_CMDTYPE_DISEQC;
#endif
		sequence->commands[sequence->numCommands].u.diseqc.addr = 0x10;
		sequence->commands[sequence->numCommands].u.diseqc.cmd = 0x39;		/* write to port group 1 (uncommitted switches) */
		sequence->commands[sequence->numCommands + 1].u.diseqc.addr = 0x10;
		sequence->commands[sequence->numCommands + 1].u.diseqc.cmd = 0x38;

		if (((sequence->numCommands == 1) && (diseqc < 8)) || ((sequence->numCommands == 3) && (diseqc >= 8)))
		{
			sequence->commands[sequence->numCommands].u.diseqc.numParams = 1;
			sequence->commands[sequence->numCommands].u.diseqc.params[0] = 0xF0 | ((diseqc << 2) & 0x0F) | (toneMode == SEC_TONE_ON ? 1 : 0) | (voltage == SEC_VOLTAGE_18 ? 2 : 0);
			sequence->commands[sequence->numCommands + 1].u.diseqc.numParams = 1;
			sequence->commands[sequence->numCommands + 1].u.diseqc.params[0] = 0xF0 | ((diseqc << 2) & 0x0F) | (toneMode == SEC_TONE_ON ? 1 : 0) | (voltage == SEC_VOLTAGE_18 ? 2 : 0);
		}
		else
		{
			sequence->commands[sequence->numCommands].u.diseqc.numParams = 0;
			sequence->commands[sequence->numCommands + 1].u.diseqc.numParams = 0;
		}
	}

	secSendSequence(sequence);

	delete sequence->commands;
	delete sequence;

	if (failed == false)
	{
		currentDiseqc = diseqc;
		return true;
	}
	else
	{
		return false;
	}
}

const bool CFrontend::sendDiseqcSmatvRemoteTuningCommand (secToneMode toneMode, secVoltage voltage, uint8_t diseqc, uint32_t frequency)
{
	secCmdSequence *sequence = new secCmdSequence();
	sequence->commands = new secCommand[2];

	sequence->miniCommand = SEC_MINI_NONE;
	sequence->continuousTone = toneMode;
	sequence->voltage = voltage;
	sequence->numCommands = 2;

	sequence->commands[0].type = SEC_CMDTYPE_DISEQC;
	sequence->commands[0].u.diseqc.addr = 0x10;	/* any interface */
	sequence->commands[0].u.diseqc.cmd = 0x38;	/* write n0 */
	sequence->commands[0].u.diseqc.numParams = 1;
	sequence->commands[0].u.diseqc.params[0] = 0xF0 | ((diseqc << 2) & 0x0F) | (toneMode == SEC_TONE_ON ? 1 : 0) | (voltage == SEC_VOLTAGE_18 ? 2 : 0);

	sequence->commands[1].type = SEC_CMDTYPE_DISEQC;
	sequence->commands[1].u.diseqc.addr = 0x71;	/* intelligent slave interface for multi-master bus */
	sequence->commands[1].u.diseqc.cmd = 0x58;	/* write channel frequency */
	sequence->commands[1].u.diseqc.numParams = 3;
	sequence->commands[1].u.diseqc.params[0] = (((frequency / 10000000) << 4) & 0xF0) | ((frequency / 1000000) & 0x0F);
	sequence->commands[1].u.diseqc.params[1] = (((frequency / 100000) << 4) & 0xF0) | ((frequency / 10000) & 0x0F);
	sequence->commands[1].u.diseqc.params[2] = (((frequency / 1000) << 4) & 0xF0) | ((frequency / 100) & 0x0F);

	secSendSequence(sequence);

	delete sequence->commands;
	delete sequence;

	if (failed == false)
	{
		currentDiseqc = diseqc;
		return true;
	}
	else
	{
		return false;
	}
}

