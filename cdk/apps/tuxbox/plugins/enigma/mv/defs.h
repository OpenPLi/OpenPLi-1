#ifndef __STRUCT_H__
#define __STRUCT_H__

#include "time.h"
#include <lib/base/estring.h>

#define ENIGMAVERSION	1090

// size of description
#define DC_MAX_DESCR_LENGTH      2020

#define MAX_PROGRAM_NAME_LENGTH 100
#define MAX_PROGRAM_DESC_LENGTH 2022
#define MAX_CHANNEL_NAME_LENGTH 30
#define MAX_STRING_LENGTH       2023
#define MAX_PATH_LENGTH         100
#define MAX_INPUT_NAME_LENGTH   30

#define MAX_PROGRAMS_PER_CHANNEL  2000
#define MAX_CHANNELS              200
#define MAX_NO_INPUTS             5
#define MAX_VERSION_STRING_LENGTH 20

enum {
	sortTypeGraphicalBouquet = 0,
	sortTypeGraphicalAlphabetical,
	sortTypeGraphicalFirstProgramTimeRemaining,
	sortTypeListBouquetThenStartTime,
	sortTypeListStartTime,

	sortTypeFirstList = sortTypeListBouquetThenStartTime,
	sortTypeLastList = sortTypeListStartTime,

	sortTypeFirstGraphical = sortTypeGraphicalBouquet,
	sortTypeLastGraphical = sortTypeGraphicalFirstProgramTimeRemaining
};

enum {
        aspectRatioEnigma,
        aspectRatioAlways169,
        aspectRatioAlways43
};


enum {
        programDataFlagFilm = 1,
        programDataFlagFromCache = 2
};


#define MV_MODE_VIEW            0
#define MV_MODE_EDIT            1
#define MV_MODE_EDIT_SBAR       2
#define MV_MODE_EDIT_IND        3


struct ProgramData {
        eString channelName, name, descr;
        time_t  startTime;
        time_t  duration;
        int flags;
	short idxFic;
	long offset;
        int getStartHour( void ) {
                tm *begin = localtime( &startTime );
                return begin->tm_hour;
        }

        // For sort
        bool operator<(const ProgramData *p1 ) {
                return ( p1->startTime < this->startTime );
        }
};


struct ViewPresentation {
        int backColour;
        int programOneBackColour;
        int programTwoBackColour;
        int filmBackColour;
        int programTimeFontSize;
        int programTimeFontFamily;
        int programTitleFontSize;
        int programTitleFontFamily;
        int programDescrFontSize;
        int programDescrFontFamily;
        int programChannelFontSize;
        int programChannelFontFamily;
        int channelHeaderBackColour;
        int channelHeaderFontSize;
        int channelHeaderFontFamily;
        int playingBackColour;
        int favouriteBackColour;
};

struct ViewFeatures {
        int showTitleBar;
        int showStatusBar;
        int noColumns;
        int timebarPeriodSeconds;
        int entriesPerColumn;
        int channelShiftType;
        int programShiftType;
        bool horizontalSep;
        bool verticalSep;
        int channelHeaderFlags;
        int statusBarFlags;
        int programBoxFlags;
        int nextView;
        int firstChannel;
        bool showElapsedBars;
        bool showChannelBar;
        int dayBarMode;
        bool showEmptyChannels;
        bool forceCursorFlag;
	int sortType;
};


struct ViewGeometry
{
        time_t widthSeconds;
        int topLeftX, topLeftY;
        int widthPixels, heightPixels;
        int headerWidthPixels;
        int channelHeightPixels;
        int channelBarWidthPixels;

        int statusBarX, statusBarY;
        int statusBarWidthPixels, statusBarHeightPixels;

	int timeLabelX, timeLabelY;
};



struct InputConf
{
        char name[MAX_INPUT_NAME_LENGTH+1];
        int enabledFlag;
};

struct Inputs {
        int storageDir;
        int detailsDir;
        struct InputConf confs[MAX_NO_INPUTS];
        int flags;
        int maxChannelsPerInput;
        int pre;
        int post;
	bool		autoDataDirsFlag;
	bool		autoReloadFlag;
};

struct Conf {
        time_t          widthSecondsChangeRate;
        time_t          startTimeChangeRate;
        int             timeBarHeight;
        char            versionString[MAX_VERSION_STRING_LENGTH+1];
        int             timerStartOffsetSeconds;
        int             timerEndOffsetSeconds;
        int             dayBarHeight;
        int             timePickerHeight;
        time_t          lastFavouriteChecksum;
        int             maxWaitForEPGSeconds;
	int		windowInitialStartOffsetSeconds;
	int		aspectRatioMode;
	unsigned int	checksum;
};

struct ViewConf {
        struct ViewGeometry             geom;
        struct ViewFeatures             feat;
        struct ViewPresentation         pres;
};


enum {
        configErrorNone,
        configErrorNotFound,
        configErrorBadFile
};


#endif
