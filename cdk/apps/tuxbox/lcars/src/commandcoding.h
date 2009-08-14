#ifndef COMMANDCODING_H
#define COMMANDCODING_H


// Classes
enum
{
	NOTHING,
	OSD,
	HARDWARE,
	RC,
	SETTINGS,
	CONTROL,
	SCAN,
	CHANNELS,
	UPDATE,
	TIMER,
	PLUGINS,
	EIT,
	FB,
	IF,
	ELSE,
	IR,
	SDT,
	TUNER,
	ZAP,
	ROTOR,
	CI,
	TELETEXT
};

// Commands
enum
{
	C_direct,
	C_Wait,
	C_Menu,
	C_Update,
	C_Timers,
	C_Add,
	C_Set,
	C_Scan,
	C_Settings,
	C_Plugins,
	C_Save,
	C_Load,
	C_Zap,
	C_Mode,
	C_Switch,
	C_Show,
	C_Hide,
	C_Channellist,
	C_Perspectives,
	C_Dump,
	C_Shutdown,
	C_Read,
	C_Var,
	C_Sub,
	C_Send,
	C_Fillbox,
	C_Puttext,
	C_Get,
	C_Tune,
	C_Stop,
/*	Rotor & Diseq commands	*/
	C_Left,
	C_Right,
	C_West,
	C_East,
	C_Burst,
	C_Tone,
	C_Circular,
	C_Manual,
	C_Automatic,
	C_Satellite,
	C_Cable,
	C_Terrestrial,
	C_Bruteforce,
/*	Extended box commands	*/
	C_Reboot,
	C_Standby
};

#endif
