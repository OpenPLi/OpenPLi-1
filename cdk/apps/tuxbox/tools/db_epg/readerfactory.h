#ifndef READERFACTORY_H
#define READERFACTORY_H

#include "iepgreader.h"

#include <map>
/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

typedef iEpgReader*(*tCreateCallback)(void);

class cReaderFactoryInst;
class cReaderFactory
{
    public:
        //Create a statically allocated instance of this, to register your reader...
        struct tReaderType
        {
            tReaderType(string n, string t, tCreateCallback c) : name(n), type(t), cb(c), inst(0)
            {
                cReaderFactory::installReader(this);
            }
            string name; //userfriendly name
            string type; //argument type
            tCreateCallback cb;
            iEpgReader* inst;
        };
        static iEpgReader* createReader(string type);
        static const std::map<string, tReaderType*>& getReaders();


    private:
        static void installReader(tReaderType *type);
        static cReaderFactoryInst *getInstance();
        static cReaderFactoryInst *instance;
        cReaderFactory(){}; //do not allow instances of this class
};

#endif
