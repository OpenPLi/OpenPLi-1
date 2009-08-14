#include <iostream>
#include "iepgreader.h"
#include "cdbiface.h"
#include "event.h"
#ifdef DEBUG
#include "epgcache.h"
#endif


/* Storing the entries to database should be factored out - but not sure where it belongs yet */
bool iEpgReader::storeData(cEvent* data, int source)
{
    bool result = false;
    cDbIface *inst = cDbIface::getInstance();

    int len = data->getEventDataLen();
    U8 *eventStruct = new U8[len];
    data->getEventData(eventStruct, len);

    bool validData = true;
#ifdef DEBUG
    eventData d((eit_event_struct*) eventStruct, len, 0);
    //For testing: the generated data should come back unmodified it is passed through enigma's eventData
    validData = (memcmp((U8*) d.get(), eventStruct, len) == 0);
#endif
    if (validData)
    {
        result = inst->insertEpgData(data->getKey(),
                                     data->getEventId(),
                                     data->getStartTime(),
                                     data->getStopTime(),
                                     source,
                                     eventStruct,
                                     len );
    }
    else
    {
        std::cout << data->getKey() << " , " << data->getEventId() << " failed test" << std::endl;
    }
    delete [] eventStruct;
    return result;
}
