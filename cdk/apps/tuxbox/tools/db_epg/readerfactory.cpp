#include "readerfactory.h"

class cReaderFactoryInst
{
    friend class cReaderFactory;
    private:
        cReaderFactoryInst() {};
        iEpgReader* createReader(string type);
        void installReader(cReaderFactory::tReaderType *type);
        std::map<string, cReaderFactory::tReaderType*> readers;
};


cReaderFactoryInst *cReaderFactory::instance = 0;
cReaderFactoryInst *cReaderFactory::getInstance()
{
    if (instance == 0)
    {
        instance = new cReaderFactoryInst();
    }
    return instance;
}

iEpgReader* cReaderFactory::createReader(string type)
{
    return getInstance()->createReader(type);
}

void cReaderFactory::installReader(tReaderType *type)
{
    //first one to come here will automatically create the factory...
    getInstance()->installReader(type);
}

const std::map<string, cReaderFactory::tReaderType*>& cReaderFactory::getReaders()
{
    return getInstance()->readers;  //not nice, but workable...
}



iEpgReader* cReaderFactoryInst::createReader(string type)
{
    //check list of available readers, and call callback method in list to
    //instantiate a new reader of this type
    std::map<string, cReaderFactory::tReaderType*>::iterator i = readers.find(type);
    if (i != readers.end())
    {
        cReaderFactory::tReaderType *r = i->second;
        if (r->inst == 0)
        {
            r->inst = r->cb();
        }
        return (r->inst);
    }
    return 0;
}

void cReaderFactoryInst::installReader(cReaderFactory::tReaderType *type)
{
    readers.insert(std::pair<string, cReaderFactory::tReaderType*>(type->type, type));
}


