#include "control.h"

control::control (osd *o, rc *r, hardware *h, settings *s, scan *s1, channels *c, eit *e, cam *c1, zap *z, tuner *t, update *u, timer *t1, plugins *p, checker *c2, fbClass *f, variables *v, ir *i, pig *p1, teletext *t2, sdt *s2, lcddisplay *l)
{
	osd_obj = o;
	rc_obj = r;
	hardware_obj = h;
	settings_obj = s;
	scan_obj = s1;
	channels_obj = c;
	eit_obj = e;
	cam_obj = c1;
	zap_obj = z;
	tuner_obj = t;
	update_obj = u;
	timer_obj = t1;
	plugins_obj = p;
	checker_obj = c2;
	fb_obj = f;
	vars = v;
	ir_obj = i;
	pig_obj = p1;
	teletext_obj = t2;
	sdt_obj = s2;
	lcd_obj = l;

	last_read.TS = -1;
	last_read.ONID = -1;
	last_read.SID = -1;

	loadMenus();
	loadModes();
	loadSubs();
	startThread();
}

void control::run()
{
	runMode(0);
}

command_class control::parseCommand(std::string cmd)
{
	command_class tmp_command;
	tmp_command.args.clear();
	tmp_command.var.clear();
	tmp_command.if_type = NOTHING;

	if (cmd[0] == 9)
		cmd = cmd.substr(1, cmd.length());

	std::string tmp_string;
	std::istringstream iss(cmd);


	getline(iss, tmp_string, ' ');
	if (tmp_string[0] == '#')
	{
		tmp_command.cmd_class = NOTHING;
	}
	else if (tmp_string == "OSD")
	{
		tmp_command.cmd_class = OSD;
	}
	else if (tmp_string == "NOOP")
	{
		tmp_command.cmd_class = NOTHING;
		return tmp_command;
	}
	else if (tmp_string == "?")
	{
		tmp_command.if_type = IF;
		tmp_command.if_value = "";
		std::string tmp_new;
		getline(iss, tmp_new, ' ');
		std::istringstream iss2(tmp_new);
		std::string name;
		std::string val;
		getline(iss2, name, '=');
		if (getline(iss2, val, '='))
			tmp_command.if_value = val;
		tmp_command.if_var = name;

		getline(iss, tmp_new, '\n');
		command_class tmp2_command = parseCommand(tmp_new);
		tmp_command.cmd_class = tmp2_command.cmd_class;
		tmp_command.command = tmp2_command.command;
		tmp_command.args = tmp2_command.args;
		tmp_command.var = tmp2_command.var;
		return tmp_command;
	}
	else if (tmp_string == "?!")
	{
		tmp_command.if_type = ELSE;
		std::string tmp_new;

		getline(iss, tmp_new, '\n');
		command_class tmp2_command = parseCommand(tmp_new);
		tmp_command.cmd_class = tmp2_command.cmd_class;
		tmp_command.command = tmp2_command.command;
		tmp_command.args = tmp2_command.args;
		tmp_command.var = tmp2_command.var;
		return tmp_command;
	}
	else if (tmp_string == "HARDWARE")
	{
		tmp_command.cmd_class = HARDWARE;
	}
	else if (tmp_string == "RC")
	{
		tmp_command.cmd_class = RC;
	}
	else if (tmp_string == "SETTINGS")
	{
		tmp_command.cmd_class = SETTINGS;
	}
	else if (tmp_string == "CONTROL")
	{
		tmp_command.cmd_class = CONTROL;
	}
	else if (tmp_string == "SCAN")
	{
		tmp_command.cmd_class = SCAN;
	}
	else if (tmp_string == "CHANNELS")
	{
		tmp_command.cmd_class = CHANNELS;
	}
	else if (tmp_string == "UPDATE")
	{
		tmp_command.cmd_class = UPDATE;
	}
	else if (tmp_string == "TIMER")
	{
		tmp_command.cmd_class = TIMER;
	}
	else if (tmp_string == "PLUGINS")
	{
		tmp_command.cmd_class = PLUGINS;
	}
	else if (tmp_string == "EIT")
	{
		tmp_command.cmd_class = EIT;
	}
	else if (tmp_string == "FB")
	{
		tmp_command.cmd_class = FB;
	}
	else if (tmp_string == "IR")
	{
		tmp_command.cmd_class = IR;
	}
	else if (tmp_string == "SDT")
	{
		tmp_command.cmd_class = SDT;
	}
	else if (tmp_string == "TUNER")
	{
		tmp_command.cmd_class = TUNER;
	}
	else if (tmp_string == "ZAP")
	{
		tmp_command.cmd_class = ZAP;
	}
	else
	{
		std::cout << "Error in Command: Unknown Commandclass (" << tmp_string << ") on String:" << std::endl << cmd << std::endl;
		exit(1);
	}




	getline(iss, tmp_string, ' ');
	if (tmp_string == "direct")
	{
		tmp_command.command = C_direct;
	}
	else if (tmp_string == "Wait")
	{
		tmp_command.command = C_Wait;
	}
	else if (tmp_string == "Menu")
	{
		tmp_command.command = C_Menu;
	}
	else if (tmp_string == "Update")
	{
		tmp_command.command = C_Update;
	}
	else if (tmp_string == "Timers")
	{
		tmp_command.command = C_Timers;
	}
	else if (tmp_string == "Add")
	{
		tmp_command.command = C_Add;
	}
	else if (tmp_string == "Plugins")
	{
		tmp_command.command = C_Plugins;
	}
	else if (tmp_string == "Set")
	{
		tmp_command.command = C_Set;
	}
	else if (tmp_string == "Scan")
	{
		tmp_command.command = C_Scan;
	}
	else if (tmp_string == "Settings")
	{
		tmp_command.command = C_Settings;
	}
	else if (tmp_string == "Save")
	{
		tmp_command.command = C_Save;
	}
	else if (tmp_string == "Load")
	{
		tmp_command.command = C_Load;
	}
	else if (tmp_string == "Zap")
	{
		tmp_command.command = C_Zap;
	}
	else if (tmp_string == "Mode")
	{
		tmp_command.command = C_Mode;
	}
	else if (tmp_string == "Switch")
	{
		tmp_command.command = C_Switch;
	}
	else if (tmp_string == "Dump")
	{
		tmp_command.command = C_Dump;
	}
	else if (tmp_string == "Channellist")
	{
		tmp_command.command = C_Channellist;
	}
	else if (tmp_string == "Perspectives")
	{
		tmp_command.command = C_Perspectives;
	}
	else if (tmp_string == "Shutdown")
	{
		tmp_command.command = C_Shutdown;
	}
	else if (tmp_string == "Read")
	{
		tmp_command.command = C_Read;
	}
	else if (tmp_string == "Var")
	{
		tmp_command.command = C_Var;
	}
	else if (tmp_string == "Sub")
	{
		tmp_command.command = C_Sub;
	}
	else if (tmp_string == "Send")
	{
		tmp_command.command = C_Send;
	}
	else if (tmp_string == "Fillbox")
	{
		tmp_command.command = C_Fillbox;
	}
	else if (tmp_string == "Get")
	{
		tmp_command.command = C_Get;
	}
	else if (tmp_string == "Tune")
	{
		tmp_command.command = C_Tune;
	}
	else if (tmp_string == "Stop")
	{
		tmp_command.command = C_Stop;
	}
	else
	{
		std::cout << "Error in Command: Unknown Command (" << tmp_string << ") on String:" << std::endl << cmd << std::endl;
		exit(1);
	}

	if (tmp_command.command != C_direct)
	{
		while(getline(iss, tmp_string, ' '))
		{
			tmp_command.args.insert(tmp_command.args.end(), tmp_string);
			if (tmp_string[0] == '%')
			{
				tmp_command.var.insert(tmp_command.var.end(), tmp_command.args.size() - 1);
			}
		}
	}
	else
	{
		getline(iss, tmp_string, '\n');
		tmp_command.args.insert(tmp_command.args.end(), tmp_string);
	}

	return tmp_command;
}

void control::dumpchannel(int channelnr)
{
	int position = (int) (channelnr / 10);

	osd_obj->createList();
	for (int i = position * 10; i < position * 10 + 10; i++)
	{
		osd_obj->addListItem(i, channels_obj->getServiceName(i));
	}
	selected_channel = channelnr;
	osd_obj->addCommand("SHOW list");
}

int control::runCommand(command_class command, bool val)
{
	switch (command.if_type)
	{
	case IF:
		{
			std::string val = "true";
			if (command.if_value != "")
				val = command.if_value;
			if (vars->getvalue(command.if_var) == val)
			{
				lastcheck = true;
			}
			else
			{
				lastcheck = false;
				return 0;
			}
		}
		break;
	case ELSE:
		if (lastcheck)
		{
			return 0;
		}
		break;
	}

	for (int i = 0; (unsigned int) i < command.var.size(); i++)
	{
		command.args[command.var[i]] = vars->getvalue(command.args[command.var[i]]);
	}

	switch (command.cmd_class)
	{
	case OSD:
		if (command.command == C_direct)
		{
			osd_obj->addCommand(command.args[0]);
		}
		else if (command.command == C_Set)
		{
			if(command.args[0] == "Vol")
			{
				std::stringstream ostr;
				ostr << "COMMAND vol set " << command.args[1] << std::ends;

				osd_obj->addCommand(ostr.str());
			}
			else if(command.args[0] == "NumberEntry")
			{
				osd_obj->addNumberEntry(atoi(command.args[1].c_str()));
			}
			else if(command.args[0] == "EPG")
			{
				if (command.args[1] == "EventName")
				{
					osd_obj->setEPGEventName(command.args[2]);
				}
				else if (command.args[1] == "EventShortText")
				{
					osd_obj->setEPGEventShortText(command.args[2]);
				}
				else if (command.args[1] == "EventExtendedText")
				{
					osd_obj->setEPGEventExtendedText(command.args[2]);
				}
				else if (command.args[1] == "Description")
				{
					osd_obj->setEPGDescription(command.args[2]);
				}
				else if (command.args[1] == "ProgramName")
				{
					osd_obj->setEPGProgramName(command.args[2]);
				}
				else if (command.args[1] == "Starttime")
				{
					osd_obj->setEPGstarttime(atoi(command.args[2].c_str()));
				}
				else if (command.args[1] == "Duration")
				{
					osd_obj->setEPGduration(atoi(command.args[2].c_str()));
				}
				else if (command.args[1] == "Linkage")
				{
					osd_obj->setEPGlinkage(atoi(command.args[2].c_str()));
				}
				else if (command.args[1] == "Audio")
				{
					osd_obj->setEPGaudio(command.args[2]);
				}
				else if (command.args[1] == "ParRating")
				{
					osd_obj->setEPGfsk(atoi(command.args[2].c_str()));
				}
			}
		}
		else if (command.command == C_Get)
		{
			if (command.args[0] == "SelectedSchedulingEvent")
			{
				std::string cmd = "%";
				cmd.append(command.args[1]);
				vars->setvalue(cmd, osd_obj->getSelectedSchedule());
			}
		}
		else if (command.command == C_Channellist)
		{
			if(command.args[0] == "Dump")
			{
				dumpchannel(channels_obj->getCurrentChannelNumber());
			}
			else if(command.args[0] == "Get")
			{
				if(command.args[1] == "Selected")
				{
					std::string cmd = "%";
					cmd.append(command.args[2]);
					vars->setvalue(cmd, selected_channel);
				}
			}
			else if(command.args[0] == "Select")
			{
				if(command.args[1] == "Current")
				{
					int position_old = (int) (selected_channel / 10);
					int position = (int) (channels_obj->getCurrentChannelNumber() / 10);
					if (position != position_old)
					{
						dumpchannel(channels_obj->getCurrentChannelNumber());
					}
					int selected = channels_obj->getCurrentChannelNumber() % 10;
					std::stringstream ostr;
					ostr << "COMMAND list select_item " << selected << std::ends;
					std::string cmd = ostr.str();
					osd_obj->addCommand(cmd);
					selected_channel = channels_obj->getCurrentChannelNumber();
				}
				else if (command.args[1] == "Next")
				{
					int newpos = selected_channel + 1;
					if (newpos >= channels_obj->numberChannels())
					{
						newpos = 0;
					}
					int position_old = (int) (selected_channel / 10);
					int position = (int) (newpos / 10);
					if (position != position_old)
					{
						dumpchannel(newpos);
					}
					int selected = newpos % 10;
					std::stringstream ostr;
					ostr << "COMMAND list select_item " << selected << std::ends;

					osd_obj->addCommand(ostr.str());
					selected_channel = newpos;
				}
				else if (command.args[1] == "Previous")
				{
					int newpos = selected_channel - 1;
					if (newpos < 0)
					{
						newpos = channels_obj->numberChannels() - 1;
					}
					int position_old = (int) (selected_channel / 10);
					int position = (int) (newpos / 10);
					if (position != position_old)
					{
						dumpchannel(newpos);
					}
					int selected = newpos % 10;
					std::stringstream ostr;
					ostr << "COMMAND list select_item " << selected << std::ends;

					osd_obj->addCommand(ostr.str());
					selected_channel = newpos;
				}
				else if (command.args[1] == "NextPage")
				{
					int newpos = selected_channel;
					newpos = newpos + 10 - newpos % 10;
					if (newpos >= channels_obj->numberChannels())
					{
						newpos = 0;
					}
					int position_old = (int) (selected_channel / 10);
					int position = (int) (newpos / 10);
					if (position != position_old)
					{
						dumpchannel(newpos);
					}
					int selected = newpos % 10;
					std::stringstream ostr;
					ostr << "COMMAND list select_item " << selected << std::ends;

					osd_obj->addCommand(ostr.str());
					selected_channel = newpos;
				}
				else if (command.args[1] == "PreviousPage")
				{
					int newpos = selected_channel;
					newpos = newpos - newpos % 10 - 1;
					if (newpos < 0)
					{
						newpos = channels_obj->numberChannels() - 1;
					}
					int position_old = (int) (selected_channel / 10);
					int position = (int) (newpos / 10);
					if (position != position_old)
					{
						dumpchannel(newpos);
					}
					int selected = newpos % 10;
					std::stringstream ostr;
					ostr << "COMMAND list select_item " << selected << std::ends;

					osd_obj->addCommand(ostr.str());
					selected_channel = newpos;
				}
			}
		}


		break;
	case SDT:
		if (command.command == C_Get)
		{
			if (command.args[0] == "NVODs")
			{
				sdt_obj->getNVODs(channels_obj);
				vars->setvalue("%NUMBER_NVODS", channels_obj->getCurrentNVODcount());
			}
		}
	case HARDWARE:
		if (command.command == C_Set)
		{
			if (command.args[0] == "RGB")
			{
				hardware_obj->setOutputMode(OUTPUT_RGB);
				settings_obj->setOutputFormat(OUTPUT_RGB);
			}
			else if (command.args[0] == "FBAS")
			{
				hardware_obj->setOutputMode(OUTPUT_FBAS);
				settings_obj->setOutputFormat(OUTPUT_FBAS);
			}
			else if (command.args[0] == "Vol")
			{
				if (command.args[1] == "Plus")
				{
					hardware_obj->vol_plus(atoi(command.args[2].c_str()));
				}
				else if (command.args[1] == "Minus")
				{
					hardware_obj->vol_minus(atoi(command.args[2].c_str()));
				}
			}
		}
		else if (command.command == C_Shutdown)
		{
			int fpfd = open("/dev/dbox/fp0", O_RDWR);
			int on_time = (int) ((timer_obj->getTime() - time(0)) / 60);
			if (timer_obj->getNumberTimer() > 0)
			{
				if (on_time < 1 && on_time > 0)
					on_time = 1;
				else
					on_time--;
			}
			else
				on_time = 0xfff;

			ioctl(fpfd, FP_IOCTL_SET_WAKEUP_TIMER, &on_time);
			ioctl(fpfd, FP_IOCTL_GET_WAKEUP_TIMER, &on_time);
			sleep(1);
			ioctl(fpfd,FP_IOCTL_POWEROFF);
			close(fpfd);
			sleep(1);
			reboot(RB_POWER_OFF);
			exit(0);
		}
		else if (command.command == C_Switch)
		{
			if (command.args[0] == "Vcr")
			{
				hardware_obj->switch_vcr();
				if (hardware_obj->vcrIsOn())
				{
					switch(hardware_obj->getVCRStatus())
					{
					case VCR_STATUS_OFF:
						hardware_obj->fnc(0);
						checker_obj->laststat = 0;
						break;
					case VCR_STATUS_ON:
						hardware_obj->fnc(2);
						checker_obj->laststat = 0;
						break;
					case VCR_STATUS_16_9:
						hardware_obj->fnc(1);
						checker_obj->laststat = 1;
						break;
					}
				}
				else
				{
					checker_obj->aratioCheck();
				}
			}
			else if (command.args[0] == "Mute")
			{
				hardware_obj->switch_mute();
			}
		}

		break;
	case RC:
		if (command.command == C_Wait)
		{
			if (command.args[0] == "Command")
			{
				rc_obj->read_from_rc();
			}
			else if (command.args[0] == "Timed")
			{
				int max_time = atoi(command.args[1].c_str());
				int act_time = time(0);
				while ((!rc_obj->command_available()) && (time(0) - act_time < max_time));
			}
		}
		break;
	case SETTINGS:
		if (command.command == C_Set)
		{
			if (command.args[0] == "RCRepeat")
			{
				settings_obj->setRcRepeat(val);
			}
			else if (command.args[0] == "SupportOldRC")
			{
				settings_obj->setSupportOldRc(val);
			}
			else if (command.args[0] == "SwitchVCR")
			{
				settings_obj->setSwitchVCR(val);
			}
		}
		if (command.command == C_Settings)
		{
			if (command.args[0] == "Save")
			{
				settings_obj->saveSettings();
			}
			else if (command.args[0] == "Load")
			{
				settings_obj->loadSettings();
			}
		}
		break;
	case CONTROL:
		if (command.command == C_Menu)
		{
			if (command.args[0] == "Mode")
			{
				openMenu(atoi(command.args[1].c_str()));
				return 1;
			}
		}
		else if (command.command == C_Mode)
		{
			leave_mode = true;
			current_mode = atoi(command.args[0].c_str());
		}
		else if (command.command == C_Wait)
		{
			sleep(atoi(command.args[0].c_str()));
		}
		else if (command.command == C_Sub)
		{
			if (command.args[0] == "Run")
			{
				runSub(command.args[1]);
			}
		}
		else if (command.command == C_Set)
		{
			if (command.args[0] == "Autostart")
			{
				if (command.args[1] == "On")
				{
					system("touch /var/etc/.lcars");
				}
				else if (command.args[1] == "Off")
				{
					system("rm /var/etc/.lcars");
				}
			}
			else if (command.args[0] == "IP")
			{
				osd_obj->createIP();
				if (command.args[1] == "BoxIP")
				{
					osd_obj->setIPn(0, settings_obj->getIP(0));
					osd_obj->setIPn(1, settings_obj->getIP(1));
					osd_obj->setIPn(2, settings_obj->getIP(2));
					osd_obj->setIPn(3, settings_obj->getIP(3));
					osd_obj->setIPDescription("Please enter IP-address!");
				}
				else if (command.args[1] == "ServerIP")
				{
					osd_obj->setIPn(0, settings_obj->getserverIP(0));
					osd_obj->setIPn(1, settings_obj->getserverIP(1));
					osd_obj->setIPn(2, settings_obj->getserverIP(2));
					osd_obj->setIPn(3, settings_obj->getserverIP(3));
					osd_obj->setIPDescription("Please enter LCARS-Server IP-address!");
				}
				else if (command.args[1] == "GatewayIP")
				{
					osd_obj->setIPn(0, settings_obj->getgwIP(0));
					osd_obj->setIPn(1, settings_obj->getgwIP(1));
					osd_obj->setIPn(2, settings_obj->getgwIP(2));
					osd_obj->setIPn(3, settings_obj->getgwIP(3));
					osd_obj->setIPDescription("Please enter gateway's IP-address!");
				}
				else if (command.args[1] == "DNSIP")
				{
					osd_obj->setIPn(0, settings_obj->getdnsIP(0));
					osd_obj->setIPn(1, settings_obj->getdnsIP(1));
					osd_obj->setIPn(2, settings_obj->getdnsIP(2));
					osd_obj->setIPn(3, settings_obj->getdnsIP(3));				
					osd_obj->setIPDescription("Please enter DNS-IP-address!");
				}
				osd_obj->addCommand("SHOW ip");
				osd_obj->addCommand("COMMAND ip position 0");
				
				unsigned short key = 0;
				int number = 0;
				do
				{
					key = rc_obj->read_from_rc();
					number = rc_obj->get_number();

					if (number != -1)
					{
						osd_obj->setIP(number);
						osd_obj->setIPNextPosition();
					}
					else if (key == RC_RIGHT)
					{
						osd_obj->setIPNextPosition();
					}
					else if (key == RC_LEFT)
					{
						osd_obj->setIPPrevPosition();
					}
				} while ( key != RC_OK && key != RC_HOME);
				
				if (key == RC_OK)
				{
					if (command.args[1] == "BoxIP")
					{
						settings_obj->setIP(osd_obj->getIPPart(0), osd_obj->getIPPart(1), osd_obj->getIPPart(2), osd_obj->getIPPart(3));
					}
					else if (command.args[1] == "ServerIP")
					{
						settings_obj->setserverIP(osd_obj->getIPPart(0), osd_obj->getIPPart(1), osd_obj->getIPPart(2), osd_obj->getIPPart(3));
						update_obj->ip[0] = osd_obj->getIPPart(0);
						update_obj->ip[1] = osd_obj->getIPPart(1);
						update_obj->ip[2] = osd_obj->getIPPart(2);
						update_obj->ip[3] = osd_obj->getIPPart(3);
					}
					if (command.args[1] == "GatewayIP")
					{
						settings_obj->setgwIP(osd_obj->getIPPart(0), osd_obj->getIPPart(1), osd_obj->getIPPart(2), osd_obj->getIPPart(3));					
					}
					if (command.args[1] == "DNSIP")
					{
						settings_obj->setdnsIP(osd_obj->getIPPart(0), osd_obj->getIPPart(1), osd_obj->getIPPart(2), osd_obj->getIPPart(3));
						
					}
				}
				osd_obj->addCommand("HIDE ip");
			}
		}
		else if (command.command == C_Var)
		{
			if (command.args[0] == "Set")
			{
				command.args[1].insert(0, "%");
				vars->setvalue(command.args[1], command.args[2]);
			}
			else if (command.args[0] == "Length")
			{
				command.args[1].insert(0, "%");
				vars->setvalue(command.args[1], command.args[2].length());
			}
			else if (command.args[0] == "Add")
			{
				command.args[1].insert(0, "%");
				vars->setvalue(command.args[1], atoi(vars->getvalue(command.args[1]).c_str()) + atoi(command.args[2].c_str()));
			}
			else if (command.args[0] == "Sub")
			{
				command.args[1].insert(0, "%");
				vars->setvalue(command.args[1], atoi(vars->getvalue(command.args[1]).c_str()) - atoi(command.args[2].c_str()));
			}
			else if (command.args[0] == "Mul")
			{
				command.args[1].insert(0, "%");
				vars->setvalue(command.args[1], atoi(vars->getvalue(command.args[1]).c_str()) * atoi(command.args[2].c_str()));
			}
			else if (command.args[0] == "Value")
			{
				if (command.args[1] == "Channelname")
				{
					command.args[3].insert(0, "%");
					vars->setvalue(command.args[3], channels_obj->getServiceName(atoi(command.args[2].c_str())));
				}
			}

		}
		else if (command.command == C_Add)
		{
			if (command.args[0] == "PIDs")
			{
				int counter = 0;
				if (channels_obj->getCurrentChannelNumber() != -1)
				{
					for (int i = 1; (unsigned int)i < command.args.size(); i++)
					{
						if (command.args[i] == "VPID")
						{
							char text[11];
							sprintf(text, "VPID: %04x", channels_obj->getCurrentVPID());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "TXT")
						{
							char text[11];
							sprintf(text, "TXT: %04x", channels_obj->getCurrentTXT());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "TS")
						{
							char text[11];
							sprintf(text, "TS: %04x", channels_obj->getCurrentTS());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "ONID")
						{
							char text[11];
							sprintf(text, "ONID: %04x", channels_obj->getCurrentONID());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "SID")
						{
							char text[11];
							sprintf(text, "SID: %04x", channels_obj->getCurrentSID());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "PMT")
						{
							char text[11];
							sprintf(text, "PMT: %04x", channels_obj->getCurrentPMT());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "PCR")
						{
							char text[11];
							sprintf(text, "PCR: %04x", channels_obj->getCurrentPCR());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "APIDs")
						{
							char text[11];
							counter++;
							for (int i = 0; i < channels_obj->getCurrentAPIDcount(); i++)
							{
								sprintf(text, "APID: %04x", channels_obj->getCurrentAPID(i));
								osd_obj->addMenuEntry(counter, text);
							}
						}
						else if (command.args[i] == "CAIDs+ECMs")
						{
							char text[11];
							counter++;
							pmt_data pmt_entry = channels_obj->getCurrentPMTdata();
							
							for (int i = 0; i < pmt_entry.ecm_counter; i++)
							{
								sprintf(text, "CAID: %04x", pmt_entry.CAID[i]);
								osd_obj->addMenuEntry(counter, text);
								sprintf(text, "ECM: %04x", pmt_entry.ECM[i]);
								osd_obj->addMenuEntry(counter, text);
							}
						}
						else if (command.args[i] == "EMM")
						{
							char text[11];
							sprintf(text, "EMM: %04x", settings_obj->getEMMpid());
							osd_obj->addMenuEntry(++counter, text);
						}
						else if (command.args[i] == "CAID")
						{
							char text[11];
							sprintf(text, "CAID: %04x", settings_obj->getCAID());
							osd_obj->addMenuEntry(++counter, text);
						}
					}
				}
			

			}
			else if (command.args[0] == "VidData")
			{
				int counter = 0;
				for (int i = 1; (unsigned int)i < command.args.size(); i++)
				{
					if (command.args[i] == "HSIZE")
					{
						char text[11];
						sprintf(text, "H_SIZE: %d", hardware_obj->getVideoSizeX());
						osd_obj->addMenuEntry(++counter, text);
					}
					else if (command.args[i] == "VSIZE")
					{
						char text[11];
						sprintf(text, "V_SIZE: %d", hardware_obj->getVideoSizeY());
						osd_obj->addMenuEntry(++counter, text);
					}
					else if (command.args[i] == "ARATIO")
					{
						char text[11];
						int value = hardware_obj->getARatio();
						if (value == 2)
							sprintf(text, "A_RATIO: 4:3");
						else if (value == 3)
							sprintf(text, "A_RATIO: 16:9");
						else
							sprintf(text, "A_RATIO: %d", value);
						osd_obj->addMenuEntry(++counter, text);
					}
					else if (command.args[i] == "ATYPE")
					{
						char text[11];
						int value = hardware_obj->getAudioType();
						if (value == 3)
							sprintf(text, "A_TYPE: MPEG");
						else if (value == 2)
							sprintf(text, "A_TYPE: Dolby Digital");
						else
							sprintf(text, "A_TYPE: %d", value);
						osd_obj->addMenuEntry(++counter, text);
					}
				}
			}
		}
		break;
	case SCAN:
		if (command.command == C_Scan)
		{
			if (command.args[0] == "Normal")
			{
				*channels_obj = scan_obj->scanChannels(NORMAL);
				channels_obj->setStuff(eit_obj, cam_obj, hardware_obj, osd_obj, zap_obj, tuner_obj, vars);
			}
			else if (command.args[0] == "Update")
			{
				scan_obj->updateChannels(channels_obj);
			}
			else if (command.args[0] == "Full")
			{
				*channels_obj = scan_obj->scanChannels(FULL);
				channels_obj->setStuff(eit_obj, cam_obj, hardware_obj, osd_obj, zap_obj, tuner_obj, vars);
			}
			else if (command.args[0] == "Bruteforce")
			{
				*channels_obj = scan_obj->scanChannels(BRUTEFORCE);
				channels_obj->setStuff(eit_obj, cam_obj, hardware_obj, osd_obj, zap_obj, tuner_obj, vars);
			}
		}
		else if (command.command == C_Read)
		{
			if (command.args[0] == "Updates")
			{
				scan_obj->readUpdates();
			}
		}
		break;
	case CHANNELS:
		if (command.command == C_Zap)
		{
			if (command.args[0] == "Number")
			{
				channels_obj->setCurrentChannel(atoi(command.args[1].c_str()));
				channels_obj->setCurrentOSDProgramInfo();
				channels_obj->receiveCurrentEIT();
			}
			else if (command.args[0] == "Last")
			{
				channels_obj->zapLastChannel();
				channels_obj->setCurrentOSDProgramInfo();
				channels_obj->receiveCurrentEIT();

			}
			else if (command.args[0] == "Down")
			{
				int cn = channels_obj->getCurrentChannelNumber();
				cn++;
				if (cn >= channels_obj->numberChannels())
					cn = 0;
				channels_obj->setCurrentChannel(cn);
				channels_obj->setCurrentOSDProgramInfo();
				channels_obj->receiveCurrentEIT();

			}
			else if (command.args[0] == "Up")
			{
				int cn = channels_obj->getCurrentChannelNumber();
				cn--;
				if (cn < 0)
					cn = channels_obj->numberChannels() - 1;
				channels_obj->setCurrentChannel(cn);
				channels_obj->setCurrentOSDProgramInfo();
				channels_obj->receiveCurrentEIT();

			}
			else if (command.args[0] == "Zap")
			{
				channels_obj->zapCurrentChannel();

				if (channels_obj->getCurrentTXT() != 0)
				{
					teletext_obj->startReinsertion(channels_obj->getCurrentTXT());
					osd_obj->addCommand("COMMAND proginfo set_teletext true");
				}
				else
				{				
					teletext_obj->stopReinsertion();
					osd_obj->addCommand("COMMAND proginfo set_teletext false");
				}
				lcd_obj->loadFromFile(DATADIR "/lcars/lcdbackground.raw");
				lcd_obj->writeToLCD();

				lcd_obj->setTextSize(10);
				std::stringstream ostr;
				ostr << ((int)channels_obj->getCurrentChannelNumber());
				lcd_obj->putText(8, 13, 0, ostr.str());
				lcd_obj->setTextSize(12);
				lcd_obj->putText(8, 33, 0, channels_obj->getCurrentServiceName());
			}
			else if (command.args[0] == "Audio")
			{
				if (command.args[1] == "Next")
				{
					int apid = channels_obj->getCurrentAudio();
					apid++;
					if (apid >= channels_obj->getCurrentAPIDcount())
						apid = 0;
					channels_obj->zapCurrentAudio(apid);
				}
				else if (command.args[1] == "Previous")
				{
					int apid = channels_obj->getCurrentAudio();
					apid--;
					if (apid < 0 )
						apid = channels_obj->getCurrentAPIDcount() - 1;
					channels_obj->zapCurrentAudio(apid);
				}
			}
		}
		else if (command.command == C_Save)
		{
			if (command.args[0] == "DVB")
			{
				channels_obj->saveDVBChannels();
			}
			else if (command.args[0] == "TS")
			{
				channels_obj->saveTS();
			}

		}
		else if (command.command == C_Perspectives)
		{
			if (command.args[0] == "Zap")
			{
				if (command.args[1] == "Next")
				{
					int curr_perspective = atoi(vars->getvalue("%CURR_PERSPECTIVE").c_str());
					curr_perspective++;
					if (curr_perspective >= channels_obj->currentNumberPerspectives())
						curr_perspective = 0;
					channels_obj->setPerspective(curr_perspective);
					vars->setvalue("%CURR_PERSPECTIVE", curr_perspective);
					vars->setvalue("%CURR_PERSPECTIVE_NAME", channels_obj->getPerspectiveName(curr_perspective));
				}
				else if (command.args[1] == "Previous")
				{
					int curr_perspective = atoi(vars->getvalue("%CURR_PERSPECTIVE").c_str());
					curr_perspective--;
					if (curr_perspective < 0)
						curr_perspective = channels_obj->currentNumberPerspectives() - 1;
					channels_obj->setPerspective(curr_perspective);
					vars->setvalue("%CURR_PERSPECTIVE", curr_perspective);
					vars->setvalue("%CURR_PERSPECTIVE_NAME", channels_obj->getPerspectiveName(curr_perspective));
				}
				else if (atoi(command.args[1].c_str()) < channels_obj->currentNumberPerspectives())
				{
					channels_obj->setPerspective(atoi(command.args[1].c_str()));
					if (channels_obj->getCurrentType() == 4)
						channels_obj->receiveCurrentEIT();
					vars->setvalue("%CURR_PERSPECTIVE", command.args[1]);
					vars->setvalue("%CURR_PERSPECTIVE_NAME", channels_obj->getPerspectiveName(atoi(command.args[1].c_str())));
				}
			}
			else if (command.args[0] == "Parse")
			{
				channels_obj->parsePerspectives();
			}

		}
		else if (command.command == C_Load)
		{
			if (command.args[0] == "DVB")
			{
				channels_obj->loadDVBChannels();
			}
			else if (command.args[0] == "TS")
			{
				channels_obj->loadTS();
			}
		}
		break;
	case UPDATE:
		if (command.command == C_Update)
		{
			if (command.args[0] == "Manual")
			{
				update_obj->run(UPDATE_MANUALFILES);
			}
			else if (command.args[0] == "Internet")
			{
				update_obj->run(UPDATE_INET);
			}

		}
		break;
	case TIMER:
		if (command.command == C_Timers)
		{
			if (command.args[0] == "Dump")
			{
				timer_obj->dumpTimer();
			}
		}
		else if (command.command == C_Set)
		{
			if (command.args[0] == "Schedule")
			{
				timer_obj->addTimer(atoi(vars->getvalue("%STARTTIME").c_str()), 2, vars->getvalue("%EVENTNAME"), atoi(vars->getvalue("%DURATION").c_str()), channels_obj->getCurrentChannelNumber(), vars->getvalue("%AUDIO"));
			}
			else if (command.args[0] == "Now")
			{
				timer_obj->addTimer(atoi(vars->getvalue("%NOWSTARTTIME").c_str()), 2, vars->getvalue("%NOWEVENTNAME"), atoi(vars->getvalue("%NOWDURATION").c_str()), channels_obj->getCurrentChannelNumber(), vars->getvalue("%NOWAUDIO"));
			}
			else if (command.args[0] == "Next")
			{
				timer_obj->addTimer(atoi(vars->getvalue("%NEXTSTARTTIME").c_str()), 2, vars->getvalue("%NEXTEVENTNAME"), atoi(vars->getvalue("%NEXTDURATION").c_str()), channels_obj->getCurrentChannelNumber(), vars->getvalue("%NEXTAUDIO"));
			}
			else if (command.args[0] == "RemoveFromMenu")
			{
				int number = osd_obj->menuSelectedIndex();
				timer_obj->rmTimer(timer_obj->getDumpedChannel(number), timer_obj->getDumpedStarttime(number));
			}
		}
		else if (command.command == C_Save)
		{
			timer_obj->saveTimer();
		}
		break;
	case PLUGINS:
		if (command.command == C_Plugins)
		{
			if (command.args[0] == "Load")
			{
				plugins_obj->loadPlugins();
			}
			else if (command.args[0] == "Dump")
			{
				for (int i = 0; i < plugins_obj->getNumberOfPlugins(); i++)
				{
					osd_obj->addMenuEntry(i + 1, plugins_obj->getName(i));
				}
			}
		}
		break;
	case EIT:
		if (command.command == C_Dump)
		{
			if (command.args[0] == "Schedule")
			{
				if (last_read.TS != channels_obj->getCurrentTS() || last_read.ONID != channels_obj->getCurrentONID() || last_read.SID != channels_obj->getCurrentSID())
				{
					eit_obj->dumpSchedule(channels_obj->getCurrentSID(), osd_obj);
					last_read.TS = channels_obj->getCurrentTS();
					last_read.ONID = channels_obj->getCurrentONID();
					last_read.SID = channels_obj->getCurrentSID();
				}
				eit_obj->dumpSchedule(channels_obj->getCurrentTS(), channels_obj->getCurrentONID(), channels_obj->getCurrentSID(), osd_obj);
			}
			else if (command.args[0] == "NextEvent")

			{
				eit_obj->dumpNextEvent(atoi(command.args[1].c_str()));
			}
			else if (command.args[0] == "PrevEvent")
			{
				eit_obj->dumpPrevEvent(atoi(command.args[1].c_str()));
			}
			else if (command.args[0] == "NextSchedComponent")
			{
				eit_obj->dumpNextSchedulingComponent();
			}
			else if (command.args[0] == "PrevSchedComponent")
			{
				eit_obj->dumpPrevSchedulingComponent();
			}
			else if (command.args[0] == "NextNextComponent")
			{
				eit_obj->dumpNextNextComponent();
			}
			else if (command.args[0] == "PrevNextComponent")
			{
				eit_obj->dumpPrevNextComponent();
			}
			else if (command.args[0] == "NextNowComponent")
			{
				eit_obj->dumpNextNowComponent();
			}
			else if (command.args[0] == "PrevNowComponent")
			{
				eit_obj->dumpPrevNowComponent();
			}
			else if (command.args[0] == "Event")
			{
				eit_obj->dumpEvent(atoi(command.args[1].c_str()));
			}

		}
		break;
	case FB:
		if (command.command == C_direct)
		{
			fb_obj->runCommand(command.args[0]);
		}
		else if (command.command == C_Fillbox)
		{
			fb_obj->fillBox(atoi(command.args[0].c_str()), atoi(command.args[1].c_str()), atoi(command.args[2].c_str()), atoi(command.args[3].c_str()), atoi(command.args[4].c_str()));
		}
		else if (command.command == C_Get)
		{
			if (command.args[0] == "Width")
			{
				command.args[1].insert(0, "%");
				int width = 0;
				for (int i = 0; (unsigned int) i < command.args[2].length(); i++)
					width += fb_obj->getWidth(command.args[2][i]);
				vars->setvalue(command.args[1], width);
			}
		}
		break;
	case IR:
		if (command.command == C_Set)
		{
			ir_obj->setDevice(command.args[0]);
		}
		else if (command.command == C_Send)
		{
			ir_obj->sendCommand(command.args[0]);
		}
		break;
	case TUNER:
		if (command.command == C_Tune)
		{
			if (command.args[0] == "Cable")
			{
				tuner_obj->tune(atoi(command.args[1].c_str()), atoi(command.args[2].c_str()));
			}
			else if (command.args[0] == "Sat")
			{
				tuner_obj->tune(atoi(command.args[1].c_str()), atoi(command.args[2].c_str()), atoi(command.args[3].c_str()), atoi(command.args[4].c_str()), atoi(command.args[5].c_str()));
			}
		}
		break;
	case ZAP:
		if (command.command == C_Stop)
		{
			if (command.args[0] == "Demux")
			{
				zap_obj->zap_allstop();				
			}
			else if (command.args[0] == "Devices")
			{
				zap_obj->close_dev();
			}
		}
		break;
	}


	return 0;
}

bool control::checkSetting(std::string var)
{
	if (var == "%SETTINGS_LETTERBOX")
	{
		return (checker_obj->get_16_9_mode() == 2);
	}
	else if (var == "%SETTINGS_PANSCAN")
	{
		return (checker_obj->get_16_9_mode() == 1);
	}
	else if (var == "%SETTINGS_CENTERCUT")
	{
		return (checker_obj->get_16_9_mode() == 0);
	}
	else if (var == "%SETTINGS_RCREPEAT")
	{
		return (settings_obj->getRCRepeat());
	}
	else if (var == "%HARDWARE_FBAS")
	{
		return (settings_obj->getOutputFormat() == OUTPUT_FBAS);
	}
	else if (var == "%HARDWARE_RGB")
	{
		return (settings_obj->getOutputFormat() == OUTPUT_RGB);
	}
	else if (var == "%SETTINGS_SUPPORTOLDRC")
	{
		return (settings_obj->getSupportOldRc());
	}
	else if (var == "%SETTINGS_SWITCHVCR")
	{
		return (settings_obj->getSwitchVCR());
	}
	else
	{
		std::cout << "Unknown variable " << var << std::endl;
		return false;
	}
}

void control::loadModes()
{
	std::ifstream inFile;
	std::vector<std::string> line;


	int fd;
	std::string path;
	path = CONFIGDIR "/lcars/modes.lcars";
	if ((fd = open(path.c_str(), O_RDONLY)) < 0)
	{
		path = DATADIR "/lcars/modes.lcars";
		if ((fd = open(path.c_str(), O_RDONLY)) < 0)
		{
			perror("modes.lcars");
			exit(1);
		}
	}
	close(fd);
	inFile.open(path.c_str());
	if (!inFile)
	{
		perror("modes.lcars");
		exit(0);
	}

	std::string tmp_string;
	while(getline(inFile, tmp_string))
		line.insert(line.end(), tmp_string);
	inFile.close();

	int i = 0;
	while ((unsigned int) i < line.size() - 1)
	{
		std::istringstream iss(line[i]);
		mode mode;
		mode.keys.clear();
		mode.init_commands.clear();
		while(line[i] == "+++")
		{
			i++;
			if (line[i] == "Title:")
			{
				i++;
				mode.title = line[i++];
				continue;
			}
			else if (line[i] == "Index:")
			{
				i++;
				mode.index = atoi(line[i++].c_str());
				continue;
			}
			else if (line[i] == "Init:")
			{
				mode.init_commands.clear();
				while(line[++i][0] == 9)
				{
					mode.init_commands.insert(mode.init_commands.end(), parseCommand(line[i]));

				}
				continue;
			}
			else if (line[i] == "Actions:")
			{
				i++;
				while(line[i] != "+++" && line[i] != "------" && (unsigned int) i < line.size() )
				{
					int val;

					if (line[i][line[i].length() - 1] == ':')
					{
						if (line[i].substr(0, 8) == "<NUMBERS")
						{
							val = rc_obj->parseKey(line[i].substr(0, line[i].length() - 1));
						}
						else
							val = rc_obj->parseKey(line[i].substr(0, line[i].length() - 1));
					}
					else
					{
						val = rc_obj->parseKey(line[i]);
					}
					commandlist tmp_commands;
					tmp_commands.clear();
					while(line[++i][0] == 9)
					{
						tmp_commands.insert(tmp_commands.end(), parseCommand(line[i]));
					}
					mode.keys[val] = tmp_commands;
				}
				continue;
			}
			else
			{
				std::cout << "Error in modes.lcars: unknown thing in line " << (i + 1)<< std::endl;
			}
		}
		modes[mode.index] = mode;
	}
}

void control::loadSubs()
{
	std::ifstream inFile;
	std::vector<std::string> line;
	subs.clear();

	int fd;
	std::string path;
	path = CONFIGDIR "/lcars/subs.lcars";
	if ((fd = open(path.c_str(), O_RDONLY)) < 0)
	{
		path = DATADIR "/lcars/subs.lcars";
		if ((fd = open(path.c_str(), O_RDONLY)) < 0)
		{
			perror("subs.lcars");
			exit(1);
		}
	}
	close(fd);
	inFile.open(path.c_str());
	if (!inFile)
	{
		perror("subs.lcars");
		exit(1);
	}

	std::string tmp_string;
	while(getline(inFile, tmp_string))
		line.insert(line.end(), tmp_string);
	inFile.close();

	int i = 0;
	while ((unsigned int) i < line.size() - 1)
	{
		std::istringstream iss(line[i]);
		if (line[i++] != "------")
		{
		}

		std::string title;
		commandlist tmp_commands;
		while(line[i] == "+++")
		{
			i++;
			if (line[i] == "Title:")
			{
				i++;
				title = line[i++];
				continue;
			}
			else if (line[i] == "Actions:")
			{
				tmp_commands.clear();
				while(line[++i][0] == 9)
				{
					tmp_commands.insert(tmp_commands.end(), parseCommand(line[i]));

				}
				continue;
			}
			else
			{
			}
		}
		subs[title] = tmp_commands;
	}
}

void control::runSub(std::string name)
{
	commandlist tmp_commands;
	tmp_commands = subs.find(name)->second;
	for (int i = 0; (unsigned int) i < tmp_commands.size(); i++)
	{
		runCommand(tmp_commands[i]);
	}
}

bool control::subAvailable(std::string sub)
{
	return (subs.count(sub) != 0);
}

void control::runMode(int modeNumber)
{
	current_mode = modeNumber;
	runMode();
}

void control::runMode()
{
	quit_modes = false;

	while(!quit_modes)
	{
		mode mode = modes.find(current_mode)->second;

		std::cout << "Running Mode #" << mode.index << " titled \"" << mode.title << "\"" << std::endl;

		if (mode.init_commands.size() > 0)
		{
			for (int i = 0; (unsigned int) i < mode.init_commands.size(); i++)
			{
				runCommand(mode.init_commands[i]);
			}
		}
		if (mode.keys.size() > 0)
		{
			leave_mode = false;
			while(!leave_mode)
			{
				unsigned short key;
				key = rc_obj->read_from_rc();
				if (mode.keys.count(key) != 0)
				{
					commandlist commands = mode.keys.find(key)->second;
					for (int i = 0; (unsigned int) i < commands.size(); i++)
					{
						runCommand(commands[i]);
					}
				}
			}
		}
	}
}

void control::loadMenus()
{
	std::ifstream inFile;
	std::vector<std::string> line;

	int fd;
	std::string path;
	path = CONFIGDIR "/lcars/menus.lcars";
	if ((fd = open(path.c_str(), O_RDONLY)) < 0)
	{
		path = DATADIR "/lcars/menus.lcars";
		if ((fd = open(path.c_str(), O_RDONLY)) < 0)
		{
			perror("menus.lcars");
			exit(1);
		}
	}
	close(fd);
	inFile.open(path.c_str());
	if (!inFile)
	{
		perror("menus.lcars");
		exit(1);
	}

	std::string tmp_string;
	while(getline(inFile, tmp_string))
		line.insert(line.end(), tmp_string);
	inFile.close();

	int i = 0;
	while ((unsigned int) i < line.size() - 1)
	{
		std::istringstream iss(line[i]);
		if (line[i++] != "------")
		{
		}

		menu menu;
		menu.entries.clear();
		while(line[i] == "+++")
		{
			i++;
			if (line[i] == "Title:")
			{
				i++;
				menu.title = line[i++];
				continue;
			}
			else if (line[i] == "Index:")
			{
				i++;
				menu.index = atoi(line[i++].c_str());
				continue;
			}
			else if (line[i] == "Init:")
			{
				menu.init_commands.clear();
				while(line[++i][0] == 9)
				{
					menu.init_commands.insert(menu.init_commands.end(), parseCommand(line[i]));

				}
				continue;
			}
			else if (line[i] == "Entries:")
			{
				i++;
				int title_count = -200;
				menu.sort.clear();
				while(((unsigned int) i < (line.size() - 1)) && line[i] != "+++")
				{
					std::string value;
					std::string description;
					std::istringstream iss(line[i]);

					getline(iss, value, ' ');
					getline(iss, description);

					int val = -1;
					int type = -1;
					string_commandlist tmp_switches;
					switch (value[0])
					{
					case '!':
						val = atoi(value.substr(1).c_str());
						type = 2;
						while(line[++i][0] == 9)
						{
							tmp_switches.insert(tmp_switches.end(), line[i].substr(1, line[i].length() - 1));
						}
						i--;
						break;
					case 'x':
						val = atoi(value.substr(1).c_str());
						type = 1;
						break;
					case '*':
						val = title_count++;
						type = 3;
						break;
					default:
						val = atoi(value.c_str());
						type = 0;
						break;
					}


					if (menu.entries.count(val) == 0)
					{
						menu_entry tmp_entry;
						tmp_entry.description = description;
						tmp_entry.type = type;
						tmp_entry.switches = tmp_switches;
						menu.entries[val] = tmp_entry;
						menu.sort.insert(menu.sort.end(), val);
					}
					else
					{
						std::map<int, menu_entry>::iterator it = menu.entries.find(val);
						it->second.description = description;
						it->second.type = type;
						it->second.switches = tmp_switches;
						menu.sort.insert(menu.sort.end(), val);
					}

					i++;
				}
				continue;
			}
			else if (line[i] == "Actions:")
			{
				i++;
				while(line[i] != "+++" && line[i] != "------" && (unsigned int) i < line.size() )
				{
					int val = atoi(line[i].c_str());
					commandlist tmp_commands;
					tmp_commands.clear();
					while(line[++i][0] == 9)
					{
						tmp_commands.insert(tmp_commands.end(), parseCommand(line[i]));
					}
					if (menu.entries.count(val) == 0)
					{
						menu_entry tmp_entry;

						tmp_entry.action_commands = tmp_commands;
						menu.entries[val] = tmp_entry;
					}
					else
					{
						std::map<int, menu_entry>::iterator it = menu.entries.find(val);
						it->second.action_commands = tmp_commands;
					}
				}
				continue;
			}
			else if (line[i] == "Values:")
			{
				i++;
				while(line[i] != "+++" && line[i] != "------" && (unsigned int) i < line.size() )
				{
					int val = atoi(line[i].c_str());
					string_commandlist tmp_commands;
					while(line[++i][0] == 9)
					{
						tmp_commands.insert(tmp_commands.end(), line[i].substr(1, line[i].length() - 1));
					}
					if (menu.entries.count(val) == 0)
					{
						menu_entry tmp_entry;

						tmp_entry.value_commands = tmp_commands;
						menu.entries[val] = tmp_entry;
					}
					else
					{
						std::map<int, menu_entry>::iterator it = menu.entries.find(val);
						it->second.value_commands = tmp_commands;
					}
				}
				continue;
			}
			else
			{
			}
		}
		menus[menu.index] = menu;
	}
}

void control::getMenu(int menuNumber)
{
	menu tmp_menu = menus[menuNumber];

	osd_obj->createMenu();
	osd_obj->setMenuTitle(tmp_menu.title);


	for (int i = 0; (unsigned int) i < tmp_menu.sort.size(); i++)
	{
		std::map<int, menu_entry>::iterator it = tmp_menu.entries.find(tmp_menu.sort[i]);
		osd_obj->addMenuEntry(it->first, it->second.description, it->second.type);
		if (it->second.type == 2)
		{
			for (int j = 0; (unsigned int) j < it->second.switches.size(); j++)
			{
				osd_obj->addSwitchParameter(i, it->second.switches[j]);
			}
			bool set = false;
			if ((int) it->second.value_commands.size() > 0)
			{

				for (int j = 0; (unsigned int)j < it->second.value_commands.size(); j++)
				{
					if (checkSetting(it->second.value_commands[j]))
					{
						set = true;
						osd_obj->setSelected(i, it->second.switches.size() - 1 - j);
					}
				}
			}
			if (!set)
				osd_obj->setSelected(i, 0);
		}
		else if (it->second.type == 1)
		{
			if (it->second.value_commands.size() > 0)
			{
				osd_obj->setSelected(i, checkSetting(it->second.value_commands[0]));
			}
		}
	}
}

void control::openMenu(int menuNumber)
{
	menu tmp_menu = menus[menuNumber];

	getMenu(menuNumber);

	vars->setvalue("%PLUGINMENU", "false");
	for (int i = 0; (unsigned int) i < tmp_menu.init_commands.size(); i++)
	{
		runCommand(tmp_menu.init_commands[i]);
	}

	osd_obj->addCommand("SHOW menu");
	osd_obj->addCommand("COMMAND menu select next");

	int key;
	do
	{
		int number = 0;
		key = rc_obj->read_from_rc();
		number = rc_obj->get_number();

		if (key == RC_DOWN)
		{
			osd_obj->selectNextEntry();
		}
		else if (key == RC_UP)
		{
			osd_obj->selectPrevEntry();
		}
		else if (key == RC_OK)
		{
			number = osd_obj->menuSelectedIndex();
		}
		else if (key == RC_RED)
		{
			number = 30;
		}
		else if (key == RC_GREEN)
		{
			number = 31;
		}
		else if (key == RC_YELLOW)
		{
			number = 32;
		}
		else if (key == RC_BLUE)
		{
			number = 33;
		}
		if (number > 0 && number < 30 && vars->getvalue("%PLUGINMENU") == "true")
		{
			if (number > plugins_obj->getNumberOfPlugins())
				continue;
			osd_obj->addCommand("HIDE menu");
			teletext_obj->stopReinsertion();
			system("rmmod avia_gt_vbi");
			if (plugins_obj->getShowPig(number - 1))
			{
				pig_obj->hide();
				pig_obj->set_size(plugins_obj->getSizeX(number - 1), plugins_obj->getSizeY(number - 1));
				pig_obj->show();
				pig_obj->set_xy(plugins_obj->getPosX(number - 1), plugins_obj->getPosY(number - 1));
			}
			plugins_obj->setfb(fb_obj->getHandle());
			plugins_obj->setrc(rc_obj->getHandle());
			int lcdfd = open("/dev/dbox/lcd0", O_RDWR);
			plugins_obj->setlcd(lcdfd);
			plugins_obj->setvtxtpid(channels_obj->getCurrentTXT());
			rc_obj->stoprc();
			plugins_obj->startPlugin(number - 1);
			if (plugins_obj->getShowPig(number - 1))
			{
				pig_obj->hide();
			}
			rc_obj->startrc();
			rc_obj->restart();
			close(lcdfd);
			osd_obj->initPalette();
			usleep(400000);
			fb_obj->clearScreen();
			system("insmod avia_gt_vbi");
			if (channels_obj->getCurrentTXT() != 0)
			{
				teletext_obj->startReinsertion(channels_obj->getCurrentTXT());
			}
			tmp_menu = menus[menuNumber];

			getMenu(menuNumber);

			for (int i = 0; (unsigned int) i < tmp_menu.init_commands.size(); i++)
			{
				runCommand(tmp_menu.init_commands[i]);
			}

			osd_obj->addCommand("SHOW menu");
			osd_obj->addCommand("COMMAND menu select next");

		}
		else if (number != -1 && number < 30)
		{
			osd_obj->selectEntry(number);

			if (tmp_menu.entries.count(number) != 0)
			{

				std::map<int, menu_entry>::iterator it = tmp_menu.entries.find(number);
				if (it->second.type == 2 || it->second.type == 1)
				{
					bool found = false;
					int i = 0;
					for (i = 0; (unsigned int) i < tmp_menu.sort.size(); i++)
					{
						if (tmp_menu.sort[i] == number)
						{
							found = true;
							break;
						}
					}
					if (!found)
						continue;

					if (it->second.type == 2)
					{
						int selected = osd_obj->getSelected(i);
						selected++;
						if ((unsigned int)selected == it->second.switches.size())
							selected = 0;

						osd_obj->setSelected(i, selected);

						runCommand(it->second.action_commands[it->second.switches.size() - selected - 1]);
					}
					else if (it->second.type == 1)
					{
						osd_obj->setSelected(i, !osd_obj->getSelected(i));
						runCommand(it->second.action_commands[0], osd_obj->getSelected(i));

					}

					osd_obj->selectEntry(number);

				}
				else if (it->second.type == 0)
				{
					int old_menu = osd_obj->menuSelectedIndex();
					osd_obj->addCommand("HIDE menu");
					for (int j = 0; (unsigned int) j < it->second.action_commands.size(); j++)
					{
						if (runCommand(it->second.action_commands[j]) == 1)
						{
							getMenu(menuNumber);
						}

					}
					osd_obj->addCommand("SHOW menu");

					std::stringstream ostr;
					ostr << "COMMAND menu select " << old_menu << std::ends;

					osd_obj->addCommand(ostr.str());
				}
			}
		}
		else if (number > 29)
		{
			if (tmp_menu.entries.count(number) == 0)
				continue;
			osd_obj->addCommand("HIDE menu");
			std::map<int, menu_entry>::iterator it = tmp_menu.entries.find(number);
			for (int j = 0; (unsigned int) j < it->second.action_commands.size(); j++)
			{
				if (runCommand(it->second.action_commands[j]) == 1)
				{
					getMenu(menuNumber);
				}
			}
			tmp_menu = menus[menuNumber];

			getMenu(menuNumber);

			for (int i = 0; (unsigned int) i < tmp_menu.init_commands.size(); i++)
			{
				runCommand(tmp_menu.init_commands[i]);
			}

			osd_obj->addCommand("SHOW menu");
			osd_obj->addCommand("COMMAND menu select next");
		}
	} while(key != RC_HOME && key != RC_RIGHT && key != RC_LEFT);
	osd_obj->addCommand("HIDE menu");
	vars->setvalue("%PLUGINMENU", "false");
}

void control::startThread()
{
	pthread_create(&thread, 0, &control::startlistening, this);
}

void *control::startlistening(void *object)
{
	control *c = (control *) object;
	while(1)
	{
		std::string sub = c->vars->waitForEvent();
		std::cerr << "Sub: " << sub << std::endl;
		if (c->subAvailable(sub))
			c->runSub(sub);
	}
	return 0;
}
