#ifndef IEPGREADER_H
#define IEPGREADER_H

#include <string>
using std::string;
/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
class cEvent;

class iEpgReader
{
    public:
        /* Load a file and converts it. Needs enigma serviceId, network id, and transponder id */
        /* Event id's are generated automatically */
        virtual bool start() = 0;

        virtual void showHelpMessage() = 0; //would love for this one to be static, but virtual static doesn't exist :)
        virtual bool parseCmdLine(int argc, char *argv[]) = 0;

        virtual ~iEpgReader() {}
    protected:
        bool storeData(cEvent* data, int source);
};

struct tChannelInfo
{
    tChannelInfo(std::string id) : channelId(id), startTime(0), stopTime(0), alreadySeen(false), unknownChannel(true) {}
    string      displayName;
    string      serviceKey;
    string      channelId;
    time_t      startTime;
    time_t      stopTime;
    bool        alreadySeen;
    bool        unknownChannel;
};




#endif
