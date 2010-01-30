#include <byteswap.h>

#include <lib/dvb/logicalchanneldescriptor.h>

LogicalChannel::LogicalChannel(const uint8_t *const buffer)
{
	serviceId = r16(&buffer[0]);
	logicalChannelNumber = r16(&buffer[2]) & 0x3fff;
}


LogicalChannel::~LogicalChannel(void)
{
}


uint16_t LogicalChannel::getServiceId(void) const
{
	return serviceId;
}


uint16_t LogicalChannel::getLogicalChannelNumber(void) const
{
	return logicalChannelNumber;
}


LogicalChannelDescriptor::LogicalChannelDescriptor(const uint8_t *const buffer)
	: Descriptor((descr_gen_t *) buffer)
{

	uint16_t bytesLeft = len - 2;
	uint16_t pos = 2;
	uint16_t loopLength = 4;

	while (bytesLeft >= loopLength)
	{
		channelList.push_back(new LogicalChannel(&buffer[pos]));
		bytesLeft -= loopLength;
		pos += loopLength;
	}
}


LogicalChannelDescriptor::~LogicalChannelDescriptor(void)
{
	for (LogicalChannelListIterator i = channelList.begin(); i != channelList.end(); ++i)
		delete *i;
}


const LogicalChannelList *LogicalChannelDescriptor::getChannelList(void) const
{
	return &channelList;
}


#ifdef SUPPORT_XML
eString LogicalChannelDescriptor::toString()
{
        eString res="<LogicalChanelDescriptor>";
	for (LogicalChannelListIterator i = channelList.begin(); i != channelList.end(); ++i)
        {
                res+=eString().sprintf("<LogicalChanelDescriptorEntry>");
                res+=eString().sprintf("<service_id>%04x</service_id>", (*i)->getServiceId());
                res+=eString().sprintf("<logical_channel_number>%04x</logical_channel_number>", (*i)->getLogicalChannelNumber());
                res+="</LogicalChanelescriptorEntry>";
        }
        res+="</LogicalChanelDescriptor>\n";
        return res;
}
#endif
