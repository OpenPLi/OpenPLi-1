#ifndef OPTIONPARSER_H
#define OPTIONPARSER_H

#include <string>

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

class OptionParserInst;

class OptionParser
{
public:
    OptionParser();
    ~OptionParser();

    /* Register an option. Either shortId or longId is required.
       If no shortId is needed, set it to 0.
       LongId must be at least 2 characters long
       Description is a short textual description of this option. It
       is used to print available options (e.g. for building  help messages)
       Returns true if registration was successful, false if an option with
       the same id, shortId or longId is already registered.
    */
    bool registerOption(int optionId,
                        char shortId,
                        const std::string& longId,
                        const std::string& desc);

    /* Print out all registered options to stdout */
    void printOptionDescriptions(bool isMain = false);

    /* Print options and their values (mostly used for debugging) */
    void printOptionValues();

    /* Parse command line, and store options which were registered before */
    bool parseOptions(int argc, char *argv[]);

    /* Check if an option was present on the command line. */
    bool getOptionFound(int id) const;

    /* Check if the option was found, and if so, checks if it has an associated value
    Returns true if option was found. Available is set to true if associated value is available
    */
    bool getOptionValAvailable(int id, bool &available) const;

    /* Retrieve an option value by Id. Returns false if value returned value is
    valid (option found on cmdLine, and conversion (if needed) succeeded
     */
    bool getOptionValue(int id, std::string &value) const;
    bool getOptionValue(int id, int &value) const;

private:
    static OptionParserInst *instance;
};


#endif
