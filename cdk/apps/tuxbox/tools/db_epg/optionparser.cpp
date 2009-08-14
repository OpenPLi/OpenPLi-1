#include "optionparser.h"
#include <map>
#include <list>
#include <set>
#include <iostream>
#include <sstream>

class OptionParserInst
{
    friend class OptionParser;
    OptionParserInst();
    ~OptionParserInst();
    public:
        bool registerOption(int optionId,
                            char shortId,
                            const std::string& longId,
                            const std::string& desc);

        void printOptionDescriptions(bool isMain);
        void printOptionValues();

        bool parseOptions(int argc, char *argv[]);

        bool getOptionFound(int id) const;
        bool getOptionValue(int id, std::string &value) const;
        bool getOptionValue(int id, int &value) const;
        bool getOptionValAvailable(int id, bool &available) const;
    private:
        bool handleOption(std::string arg);
        bool handleLongOption(std::string arg);
        bool handleShortOption(std::string arg);
        void handleOptionValue(std::string val);

        struct tOptionInfo
        {
            tOptionInfo(char s, const std::string& l, const std::string& d, int id)
                : shortId(s), longId(l), description(d), optionId(id) {}
            char        shortId;
            std::string longId;
            std::string description;
            int optionId;
        };
        std::map<int, tOptionInfo>  optionIds;
        std::map<char, int>         shortSelMap;
        std::map<std::string, int>  longSelMap;

        struct tOptVal
        {
            bool        valAvailable;
            std::string val;
        };
        std::map<int, tOptVal>  optValues;

        int  currentOptionId;
};

/* Static instance, gets created once, and never deleted */
OptionParserInst *OptionParser::instance = 0;


OptionParser::OptionParser()
{
    if (instance == 0)
    {
        instance = new OptionParserInst();
    }
}


OptionParser::~OptionParser()
{
}

bool OptionParser::registerOption(int optionId,
                                  char shortId,
                                  const std::string& longId,
                                  const std::string& desc)
{
    return instance->registerOption(optionId, shortId, longId, desc);
}

void OptionParser::printOptionDescriptions(bool isMain)
{
    instance->printOptionDescriptions(isMain);
}
/* Parse command line, and store options which were registered before
Return value indicates if internal state is still consistent
 */
bool OptionParser::parseOptions(int argc, char *argv[])
{
    return instance->parseOptions( argc, argv);
}

/* Retrieve an option value by Id. Returns false if value returned value is
valid (option found on cmdLine, and conversion (if needed) succeeded
*/
bool OptionParser::getOptionValue(int id, std::string &value) const
{
    return instance->getOptionValue(id, value);
}

bool OptionParser::getOptionValue(int id, int &value) const
{
    return instance->getOptionValue(id, value);
}

bool OptionParser::getOptionFound(int id) const
{
    return instance->getOptionFound(id);
}

bool OptionParser::getOptionValAvailable(int id, bool &available) const
{
    return instance->getOptionValAvailable(id, available) ;
}


/*********************************/
/* Actual implementation follows */

OptionParserInst::OptionParserInst()
    : currentOptionId(0)
{
}
bool OptionParserInst::registerOption(int optionId,
                                      char shortId,
                                      const std::string& longId,
                                      const std::string& desc)
{
    if (optionIds.find(optionId) != optionIds.end())
        return false;

    if (shortId == 0 && longId == "")
        return false;
    if (shortId != 0 && (shortSelMap.find(shortId) != shortSelMap.end()))
        return false;
    if (longId != "" && (longSelMap.find(longId) != longSelMap.end()))
        return false;

    optionIds.insert(std::pair<int, tOptionInfo>(optionId, tOptionInfo(shortId, longId, desc, optionId)));
    shortSelMap[shortId] = optionId;
    longSelMap[longId] = optionId;
    //optionVals[id] will contain string value...
    return true;
}

void OptionParserInst::printOptionDescriptions(bool isMain)
{
    for (std::map<int, tOptionInfo>::const_iterator it=optionIds.begin(); it != optionIds.end(); ++it)
    {
        std::stringstream ss;
        const tOptionInfo& info = it->second;
        if ((isMain && info.optionId < 0) || (!isMain && info.optionId >= 0))
        {
            int curPos = ss.tellp();
    
            bool first = true;
            if (info.shortId != 0)
            {
                ss << " -" << info.shortId;
                first = false;
            }
            if (info.longId != "")
            {
                if (!first) ss << ",";
                ss << " --" << info.longId;
            }
            std::string::size_type pos = info.description.find('=');
            std::string arg;
            std::string description = info.description;
            if (pos != std::string::npos)
            {
                arg = description.substr(0, pos);
                description = description.substr(pos+1);
            }
            ss << " " << arg;
            curPos = ss.tellp();
            ss.width(40 - curPos);
            //ss.fill(' ');
            ss.setf(std::ios_base::right);
            ss << ":";
            ss.setf(std::ios_base::left);
            ss << description;
            std::cout << ss.str() << std::endl;
        }
    }
}
void OptionParserInst::printOptionValues()
{
    std::cout << " not implemented yet " << std::endl;
}

/* Parse command line, and store options which were registered before
Return value indicates if internal state is still consistent
 */
bool OptionParserInst::parseOptions(int argc, char *argv[])
{
    //check each argv, if it starts with a '-',consider it an option
    //this means that option values which start with '-' must be quoted!
    //Sometime when I'm feeling really borde, I'll try to create a proper state machine (HSM)
    bool expectVal = false;
    for (int i=0; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            expectVal =  handleOption(argv[i]);
        }
        else if (expectVal)
        {
            handleOptionValue(argv[i]);
            expectVal = false;
        }
        //else if not an option, and we're not expecting a value, just ignore it...
    }
    return true;
}

bool OptionParserInst::handleOption(std::string arg)
{
    bool result = false;
    if (arg.length() > 1)
    {
        //We have at least one valid character...
        switch (arg[1])
        {
            case '-':
                result = handleLongOption(arg);
                break;
            default:
                result = handleShortOption(arg);
                break;
        }
    }
    return result;
}
/* */
bool OptionParserInst::handleShortOption(std::string arg)
{
    bool expectVal = false;
    if (arg.length() > 1)
    {
        arg = arg.substr(1);
        std::string val;
        bool haveValue = false;

        //check if there is a '=' somewhere
        std::string::size_type equalPos = arg.find('=');
        if (equalPos != std::string::npos)
        {
            //either we have a value, or this is garbage....
            val = arg.substr(equalPos+1, 0);
            arg = arg.substr(0, equalPos);
            haveValue = true;
        }
        //get Id for this option...
        std::map<char, int>::const_iterator it = shortSelMap.find(arg[0]);
        if (it != shortSelMap.end())
        {
            currentOptionId = it->second;
            optValues[currentOptionId].valAvailable = false;
            expectVal = true;

            if (haveValue && val.length() > 0)
            {
                handleOptionValue(val);
                expectVal = false;
            }
        }
    }

    return expectVal;
}

bool OptionParserInst::handleLongOption(std::string arg)
{
    bool expectVal = false;
    if ((arg.length() > 3) && (arg.find("--") == 0))   //requirement: '--' + at least 2 characters
    {
        arg = arg.substr(2);
        std::string val;
        bool haveValue = false;

        //check if there is a '=' somewhere
        std::string::size_type equalPos = arg.find('=');
        if (equalPos != std::string::npos)
        {
            //either we have a value, or this is garbage....
            val = arg.substr(equalPos+1);
            arg = arg.substr(0, equalPos);
            //trim enclosing '"'
            //removeQuotes
            haveValue = true;
        }
        //get Id for this option...
        std::map<std::string, int>::const_iterator it = longSelMap.find(arg);
        if (it != longSelMap.end())
        {
            currentOptionId = it->second;
            optValues[currentOptionId].valAvailable = false;
            expectVal = true;

            if (haveValue && val.length() > 0)
            {
                handleOptionValue(val);  //if !expectVal -> unknown option...
                expectVal = false;
            }
        }
    }
    return expectVal;
}


void OptionParserInst::handleOptionValue(std::string val)
{
    if (val.length() > 0)
    {
        optValues[currentOptionId].val = val;
        optValues[currentOptionId].valAvailable = true;
    }
}

/* Retrieve an option value by Id. Returns false if value returned value is
valid (option found on cmdLine, and conversion (if needed) succeeded
*/
bool OptionParserInst::getOptionValue(int id, std::string &value) const
{
    std::map<int, tOptVal >::const_iterator it=optValues.find(id);
    if (it != optValues.end())
    {
        value = it->second.val;
        return true;
    }
    return false;
}

bool OptionParserInst::getOptionValue(int id, int &value) const
{
    std::map<int, tOptVal>::const_iterator it=optValues.find(id);
    if (it != optValues.end())
    {
        if (it->second.valAvailable)
        {
            int res = atoi(it->second.val.c_str());
            value = res;
        }
        return it->second.valAvailable;
    }
    return false;
}

bool OptionParserInst::getOptionFound(int id) const
{
    std::map<int, tOptVal>::const_iterator it=optValues.find(id);
    if (it != optValues.end())
        return true;

    return false;
}

bool OptionParserInst::getOptionValAvailable(int id, bool &available) const
{
    std::map<int, tOptVal>::const_iterator it=optValues.find(id);
    if (it != optValues.end())
    {
        available = it->second.valAvailable;
        return true;
    }

    return false;
}
