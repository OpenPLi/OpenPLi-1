/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.
*/
/*************************************************************/

#ifndef __TEXT_H__
#define __TEXT_H__

#include "lib/base/estring.h"

#include "util.h"
#include "conf.h"

enum {
	strVersion = 0,
	strWelcomeText,
	strButtonDone,
	strButtonAdd,
	strButtonDelete,
	strButtonEdit,
	strButtonUnknown,
	strButtonMapped,
	strButtonIgnore,

	// Feature window sections
	strFeaturedEditMenuGlobalView,
        strFeaturedEditMenuChannels,
        strFeaturedEditMenuProgramms,
        strFeaturedEditMenuDescWindow,
	strFeaturedEditMenuEPGStyle,

	// epg styles
        strEPGStyle1,
        strEPGStyle2,
        strEPGStyle3,
        strEPGStyle4,
        strEPGStyle5,
        strEPGStyle6,
        strEPGStyle7,
        strEPGStyle8,
        strEPGStyle9,

	strButtonPresMenuBackground,
	strButtonPresMenuFontSize,
	strButtonPresMenuFontFamily,
	strButtonPresMenuBackOne,
	strButtonPresMenuBackTwo,
	strButtonPresMenuPlaying,
	strButtonPresMenuFilm,
	strButtonPresMenuFavourite,

	strLabelPresMenuChannelHeader,
	strLabelPresMenuProgramBox,
	strLabelPresMenuWindow,

	strPresMenuTestChannelHeader,
	strPresMenuTestChannelProgOne,
	strPresMenuTestChannelProgTwo,
	strPresMenuTestChannelCursor,
	strPresMenuTestChannelFavourite,
	strPresMenuTestChannelFilm,

	// MISC MENU

	strLabelMiscMenuTimerStartOffset,
	strLabelMiscMenuTimerEndOffset,
	strLabelMiscMenuViewOffset,

	strLabelMiscMenuAspectRatio,
	strMiscMenuAspectRatioEnigma,
	strMiscMenuAspectRatio169,
	strMiscMenuAspectRatio43,

	strMiscMenuAspectRatioWarning,

	// FEATURE MENU

	strLabelFeatMenuTimeBar,
	strFeatMenuTimeBarLabelOff,
	strFeatMenuTimeBarLabel15Minutes,
	strFeatMenuTimeBarLabel30Minutes,
	strFeatMenuTimeBarLabel60Minutes,
	strFeatMenuTimeBarLabel120Minutes,

	strLabelFeatMenuDayBar,
	strFeatMenuDayBarLabelOff,
	strFeatMenuDayBarLabelFixed,
	strFeatMenuDayBarLabelPopup,

	strLabelFeatMenuNoCols,
	strLabelFeatMenuItemsPerCol,
	strLabelFeatMenuItemsPerChannel,
	strFeatMenuItemsPerColProportional,
	strFeatMenuItemsPerColAll,

	strLabelFeatMenuChannelShiftType,
	strFeatMenuChannelShiftTypeOneChannel,
	strFeatMenuChannelShiftTypeHalfPage,
	strFeatMenuChannelShiftTypeOnePage,


	strLabelFeatMenuProgramShiftType,
	strFeatMenuProgramShiftTypeOneProgram,
	strFeatMenuProgramShiftTypeHalfPage,
	strFeatMenuProgramShiftTypeOnePage,

	strCheckboxFeatMenuProgramBoxStart,
	strCheckboxFeatMenuProgramBoxEnd,
	strCheckboxFeatMenuProgramBoxDuration,
	strCheckboxFeatMenuProgramBoxChannel,
	strCheckboxFeatMenuProgramBoxDifference,
	strCheckboxFeatMenuProgramBoxDescription,
	strCheckboxFeatMenuProgramBoxChannelIcon,
	strCheckboxFeatMenuProgramBoxCentreTitle,

	strCheckboxFeatMenuStatusBar,
	strCheckboxFeatMenuStatusBarChannel,
	strCheckboxFeatMenuStatusBarDate,
	strCheckboxFeatMenuStatusBarTime,
	strCheckboxFeatMenuStatusBarDuration,
	strCheckboxFeatMenuStatusBarProgram,
	strCheckboxFeatMenuStatusBarDescription,

	strCheckboxFeatMenuChannelHeaderName,
	strCheckboxFeatMenuChannelHeaderIcon,
	strCheckboxFeatMenuChannelHeaderNumber,
	strCheckboxFeatMenuChannelHeaderSatPos,

	strCheckboxFeatMenuElapsed,
	strCheckboxFeatMenuVSep,
	strCheckboxFeatMenuHSep,
	strCheckboxFeatMenuEmptyChannels,
	strCheckboxFeatMenuForceCursor,

	strLabelFeatMenuStartChannel,
	strFeatMenuStartChannelPlaying,
	strFeatMenuStartChannelTop,

	strFeatMenuNextViewThis,

	strLabelFeatMenuSort,
	strFeatMenuSortGraphicalBouquet,
	strFeatMenuSortGraphicalAlphabetic,
	strFeatMenuSortGraphicalFirstProgramTimeRemaining,
	strFeatMenuSortListBouquetChannelThenStartTime,
	strFeatMenuSortListStartTime,

	// INPUT MENU

	strLabelInputMenuHeaderInput,
	strLabelInputMenuHeaderLocalDir,
	strLabelInputMenuHeaderDownload,

	strLabelInputMenuNetworkNotNeeded,
	strLabelInputMenuNetworkNeeded,
	strLabelInputMenuNetworkNeededNoNetwork,

	strLabelInputMenuPre,
	strLabelInputMenuPost,
	strLabelInputMenuDetailStore,
	strLabelInputMenuStorageDir,
	strCheckboxFeatureMenuAutoDirs,
	strLabelInputMenuMaxChannels,
	strCheckboxFeatureMenuAutoReload,

	// FAV EDIT

	strLabelFavEditTitle,
	strFavEditBoolAny,
	strFavEditBoolAll,
	strFavEditBoolAllInOrder,
	strFavEditBoolNone,

	strLabelFavEditDescr,
	strLabelFavEditChannel,
	strLabelFavEditTimeBetween,
	strLabelFavEditTimeOutside,
	strLabelFavEditTimeAnd,

	strCheckBoxFavEditNotify,

	// MORE MISC

	strDownloadingMessage,
	strXMLTVConvertMessage,
	strViewHelpTitle,
	strViewHelp,
	strEditHelpTitle,
	strEditHelp,
	strProgramListHelpTitle,
	strProgramListHelp,
	strProgramListEditHelpTitle,
	strProgramListEditHelp,
	strAbout,
	strFeedbackTitle,
	strFeedback,
	strTipsTitle,
	strTips,
	strDaybarSunday,
	strDaybarMonday,
	strDaybarTuesday,
	strDaybarWednesday,
	strDaybarThursday,
	strDaybarFriday,
	strDaybarSaturday,
	strMainMenuTitle,
	strMainMenuItemViewGeom,
	strMainMenuItemViewPres,
	strMainMenuItemViewFeat,
	strMainMenuItemInputConfig,
	strMainMenuItemAliasManager,
	strMainMenuItemFavManager,
	strMainMenuItemMiscSettings,
	strMainMenuItemAbout,
	strMainMenuItemTips,
	strMainMenuItemFeedback,
	strMainMenuItemReset,
	strMiscMenuTitle,
	strAliasManagerTitle,
	strFavManagerTitle,
	strFavEditTitle,
	strFeaturesWindowTitle,
	strInputsWindowTitle,
	strPresentationWindowTitle,
	strChannelPickerWindowTitle,
	strProgramListWindowTitlePrefix,
	strConfirmTitle,
	strResetConfirmText,
	strMyXMLTVConfirmText,

	strIMDBNoRecords,
	strIMDBNotFilm,

	strOne,
	strTwo,
	strThree,
	strFour,
	strFive,
	strSix,
	strSeven,
	strEight,
	strNine,

	strDeletingOldConfig,
	strDownloadInProgress,

	strSystemTimeBad,
	strErrWriteOpenConf,
	strErrWritingConf,
	strOOM,
	strErrNoNetwork,

	strErrXMLParse,
	strErrXMLNoRootNode,
	strErrXMLBadRootNode,

	strErrDownload,
	strErrDownloadFailedRenameTempfile,
	strErrDownloadFilesizeZero,
	strErrDownloadNoFile,

	strErrDeletingOldFiles,
	strErrWritingFavourites,
	strErrBadFavouritesLine,
	strErrWriteOpenInputDefs,
	strErrReadOpenInputDefs,
	strErrBadInputDefsLine,

	strErrWritingChannelMap,
	strErrMakingCacheDir,
	strErrInputNameHasNoDefinition,
	strErrBadInputType,
	strErrBadInputFormat,
	strErrXMLTVConversionMovingTempFile,
	strErrMakingDir,
	strErrMovingOldCacheToTempFile,
	strErrOpeningCacheBackupFile,
	strErrDeletingTempCacheBackupFile,
	strErrCouldntSetModifyTime,
	strErrBadCharacterEncoding,

	strErrAddingToDetailCache,

	strNoStrings,

	// These are just starting items of groups,
	// don't exist in lang file

	strDaybarFirst = strDaybarSunday,
	strMainMenuItemFirst = strMainMenuItemViewGeom,
	strFeatMenuTimeBarLabelFirst = strFeatMenuTimeBarLabelOff,
	strFeatMenuDayBarLabelFirst = strFeatMenuDayBarLabelOff,
	strFeatMenuChannelShiftTypeFirst = strFeatMenuChannelShiftTypeOneChannel,
	strFeatMenuProgramShiftTypeFirst = strFeatMenuProgramShiftTypeOneProgram,
	strFeatMenuStartChannelFirst = strFeatMenuStartChannelPlaying,
	strMiscMenuAspectRatioFirst = strMiscMenuAspectRatioEnigma,
	strFavEditBoolFirst = strFavEditBoolAny,

	strFeatMenuSortFirst = strFeatMenuSortGraphicalBouquet,
	strFeaturedMenuFirst = strFeaturedEditMenuGlobalView
};

bool initStrings( char *langCode, bool moanFlag = false );
char * getStr( unsigned int strNo );

#endif

