/*
 * $Id: frontend.h,v 1.16 2002/10/12 20:19:44 obi Exp $
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

#ifndef __frontend_h__
#define __frontend_h__

/* system */
#include <stdint.h>

/* nokia api */
#include <ost/frontend.h>
#include <ost/sec.h>

/* zapit */
#include "channel.h"

#define MAX_LNBS	4

class CFrontend
{
	private:
		/* frontend file descriptor */
		int frontend_fd;
		/* sec file descriptor */
		int sec_fd;
		/* last action failed flag */
		bool failed;
		/* tuning finished flag */
		bool tuned;
		/* all devices opened without error flag */
		bool initialized;
		/* information about the used frontend type */
		FrontendInfo *info;
		/* current tuned transport stream id / original network id */
		uint32_t currentTsidOnid;
		/* current tuned frequency */
		uint32_t currentFrequency;
		/* current 22kHz tone mode */
		secToneMode currentToneMode;
		/* current H/V voltage */
		secVoltage currentVoltage;
		/* current diseqc position */
		uint8_t currentDiseqc;
		/* how often to repeat DiSEqC 1.1 commands */
		uint32_t diseqcRepeats;
		/* DiSEqC type of attached hardware */
		diseqc_t diseqcType;
		/* low lnb offsets */
		int32_t lnbOffsetsLow[MAX_LNBS];
		/* high lnb offsets */
		int32_t lnbOffsetsHigh[MAX_LNBS];

	public:
		CFrontend ();
		~CFrontend ();

		/* ost tuner api */
		static CodeRate CFrontend::getFEC (uint8_t FEC_inner);
		static Modulation CFrontend::getModulation (uint8_t modulation);

		void selfTest ();
		void setPowerState (FrontendPowerState state);
		void setFrontend (FrontendParameters *feparams);
		const FrontendPowerState getPowerState ();
		const FrontendStatus getStatus ();
		const uint32_t getBitErrorRate ();
		const int32_t getSignalStrength ();
		const int32_t getSignalNoiseRatio ();
		const uint32_t getUncorrectedBlocks ();
		const uint32_t getNextFrequency (uint32_t frequency);
		const uint32_t getNextSymbolRate (uint32_t rate);
		const FrontendParameters *getFrontend ();
		const bool getEvent ();
		const FrontendInfo *getInfo ()	{ return info; };
		unsigned int getFrequency ();
		unsigned char getPolarization ();

		/* ost sec api */
		void secSetTone (secToneMode mode);
		void secSetVoltage (secVoltage voltage);
		void secSendSequence (secCmdSequence *sequence);
		//void secResetOverload ();
		const secStatus *secGetStatus ();

		/* zapit tuner api */
		const bool tuneChannel (CZapitChannel *channel);
		const bool tuneFrequency (FrontendParameters feparams, uint8_t polarization, uint8_t diseqc);

		/* zapit diseqc api */
		const bool sendDiseqcMiniCommand (secToneMode mode, secVoltage voltage, uint8_t diseqc);
		const bool sendDiseqcCommand (secToneMode mode, secVoltage voltage, uint8_t diseqc, uint32_t repeats);
		const bool sendDiseqcPowerOn ();
		const bool sendDiseqcReset ();
		const bool sendDiseqcStandby ();
		const bool sendDiseqcZeroByteCommand (uint8_t addr, uint8_t cmd);
		const bool sendDiseqcSmatvRemoteTuningCommand (secToneMode toneMode, secVoltage voltage, uint8_t diseqc, uint32_t frequency);

		void setDiseqcRepeats(uint32_t repeats)	{ diseqcRepeats = repeats; }
		void setDiseqcType(diseqc_t type)	{ diseqcType = type; }
		const uint32_t getDiseqcRepeats()	{ return diseqcRepeats; }
		const diseqc_t getDiseqcType()		{ return diseqcType; }
		const bool isInitialized()		{ return initialized; }
		const uint32_t getTsidOnid()		{ return currentTsidOnid; }

		void setLnbOffset(bool high, uint8_t index, int32_t offset)
		{
			if (index < MAX_LNBS)
			{
				if (high)
				{
					lnbOffsetsHigh[index] = offset;
				}
				else
				{
					lnbOffsetsLow[index] = offset;
				}
			}
		}
};

#endif /* __frontend_h__ */
