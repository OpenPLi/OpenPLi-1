#ifndef __MYISO639_H__
#define __MYISO639_H__

struct ISO639
{
      char *iso639foreign, *iso639int, *twoletterm, *description1, *
description2;
} myIso639[]=
	{
		{"ces", "cze", "cs", "Czech", "Slavic"},
		{"deu", "ger", "de", "German", "Germanic"},
		{"ell", "gre", "el", "Greek, Modern (1453-)", "Latin/greek"},
		{"eng", "eng", "en", "English", "Germanic"},
		{"fin", "fin", "fi", "Finnish", "Finno-ugric"},
		{"fra", "fre", "fr", "French", "Romance"},
		{"hrv", "scr", "hr", "Croatian", "Slavic"},
		{"hun", "hun", "hu", "Hungarian", "Finno-ugric"},
		{"ita", "ita", "it", "Italian", "Romance"},
		{"nld", "dut", "nl", "Dutch", "Germanic"},
		{"nor", "nor", "no", "Norwegian", "Germanic"},
		{"pol", "pol", "pl", "Polish", "Slavic"},
		{"por", "por", "pt", "Portuguese", "Romance"},
		{"ron", "rum", "ro", "Romanian", "Romance"},
		{"rus", "rus", "ru", "Russian", "Slavic"},
		{"slk", "slo", "sk", "Slovak", "Slavic"},
		{"slv", "slv", "sl", "Slovenian", "Slavic"},
		{"spa", "spa", "es", "Spanish", "Romance"},
		{"swe", "swe", "sv", "Swedish", "Germanic"},
		{"tur", "tur", "tr", "Turkish", "Turkic/altaic"}
	};

#endif
