#include "xmlparser.h"
#include <iostream>

cXmlTvParser::cXmlTvParser(const std::string& encoding)
    : XML_Parser(encoding.c_str())
    , callback(0)
{
    root = 0;
    current = 0;
    startElementHandler=1;
    characterDataHandler=1;
    endElementHandler=1;

    cdata.reserve(2000);
}


cXmlTvParser::~cXmlTvParser()
{
}

void cXmlTvParser::installCallbackHandler(cXmlHandlerCb* cb)
{
    callback = cb;
}

void cXmlTvParser::parseFile(const std::string& file)
{
    FILE *in = fopen( file.c_str(), "rt" );

    if (in)
    {
        bool done;
        do
        {
            char buf[4096];
            unsigned int len=fread(buf, 1, sizeof(buf), in);
            done = ( len < sizeof(buf) );
            if ( ! Parse( buf, len, done ) )
            {
                std::cout << "XMLTV parse error: " <<  file << ": " << ErrorString(GetErrorCode()) << " at line " << GetCurrentLineNumber() << std::endl;
                fclose(in);
                return ;//false;
            }
        } while (!done);
        fclose(in);
    }
    return ;//true;
}


void cXmlTvParser::StartElementHandler(const XML_Char *name, const XML_Char **atts)
{
    XMLTreeNode     *n;
    const XML_Char **a=atts;

    n=new XMLTreeNode(current, (char *) name);

    if (a)
    {
        while (*a) { n->SetAttribute((char *) a[0], (char *) a[1]); a+=2; };
    }

    /* root == current
       
    */
    if (current)
    {
        n->SetPDataOff(current->GetDataSize());
        if (current != root)
            current->AddNode(n, XMLTreeNode::ADD_CHILD);

        current=n;
    }
    else
    {
        root=current=n;
    }
}

void cXmlTvParser::EndElementHandler(const XML_Char *name)
{
    if (current)
    {
        XMLTreeNode *parent = current->GetParent();
        if (parent == root)    //this was firstlevel
        {
            //invoke callback and clear this item
            if (callback) callback->OnNewNode(current);
            delete current;
        }
        current = parent;
    }
}

void cXmlTvParser::CharacterDataHandler(const XML_Char *s, int len)
{
    if (current)
        current->AppendData((char *) s, len);
}


