#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <xmltree.h>
#include <xmlparse.h>
#include <string>
#include <map>
/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

class cXmlTvParser : public XML_Parser
{
public:
    class cXmlHandlerCb
    {
        public:
//             virtual void OnStart(const std::string& name, const std::map<std::string, std::string>& attributes) = 0;
//             virtual void OnData(const std::string& data) = 0;
//             virtual void OnEnd(const std::string& name) = 0;
            virtual void OnNewNode(XMLTreeNode *node) = 0;
    };

    cXmlTvParser(const std::string& encoding);

    ~cXmlTvParser();

    void installCallbackHandler(cXmlHandlerCb* cb);

    void parseFile(const std::string& file);

protected:
    XMLTreeNode *root;
    XMLTreeNode *current;
    XMLTreeNode *firstLevel;
    void StartElementHandler(const XML_Char *name, const XML_Char **atts);
    void EndElementHandler(const XML_Char *name);
    void CharacterDataHandler(const XML_Char *s, int len);

private:
    cXmlHandlerCb *callback;
    /* what is an efficient store for cdata? needs to grow, dynamically, only add at end, no random access, copy from 1st to last
    list like, single ended, only forward iterators

    vector: resize?

    AH well, untill I figure that out, use a std::string, but reserve some space up front. Hopefully this prevents reallocation etc..
     */
    std::string cdata;
    
};

#endif
