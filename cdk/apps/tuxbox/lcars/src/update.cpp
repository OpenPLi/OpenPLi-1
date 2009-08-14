#include "update.h"

update::update(osd *o, rc *r, settings *s)
{
	curl_global_init(CURL_GLOBAL_ALL);

	osd_obj = o;
	rc_obj = r;
	settings_obj = s;
}

void update::eraseall()
{
	mtd_info_t meminfo;
	int fd;
	erase_info_t erase;

	std::stringstream ostr;
	ostr << "/dev/mtd/" << cramfsmtd << std::ends;
	std::string mtd_device = ostr.str();

	if( (fd = open( mtd_device.c_str(), O_RDWR )) < 0 )
	{
		perror("mtd_open");
		exit( 1 );
	}


	if( ioctl( fd, MEMGETINFO, &meminfo ) != 0 )
	{
		perror("mtdmemgetinfo");
		exit( 1 );
	}

	erase.length = meminfo.erasesize;

	for (erase.start = 0; erase.start < meminfo.size; erase.start += meminfo.erasesize)
	{
		std::stringstream ostr;

		ostr << "Erasing: " << (erase.start*100/meminfo.size) << "% completed" << std::ends;
		osd_obj->setPerspectiveName(ostr.str());
		osd_obj->addCommand("SHOW perspective");

		if(ioctl( fd, MEMERASE, &erase) != 0)
		{
			perror("mtdmemerase");
			//exit( 1 );
		}
	}

}

void update::run(int type)
{
	system("umount /var/tmp/tmpcramfs");
	std::string mtd_block_device_mount;
	{
		std::stringstream ostr;
		ostr << "mount /dev/mtdblock/" << cramfsmtd << " /var/tmp/tmpcramfs" << std::ends;
		mtd_block_device_mount= ostr.str();
	}

	osd_obj->clearScreen();
	osd_obj->createPerspective();
	if (system("rm -rf /var/tmp/newcramfs") != 0)
	{
		osd_obj->setPerspectiveName("Error removing newcramfs.");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
		return;
	}

	mkdir("/var/tmp", (mode_t)0755);
	mkdir("/var/tmp/tmpcramfs", (mode_t)0755);
	if (system(mtd_block_device_mount.c_str()) != 0)
	{
		osd_obj->setPerspectiveName("Error mounting CramFS (wrong MTD?).");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
		return;
	}
	mkdir("/var/tmp/newcramfs", (mode_t)0755);
	mkdir("/var/tmp/newcramfs/update", (mode_t)0755);
	{
		std::stringstream ostr;
		ostr << "Copying files, this may take a while" << std::ends;
		osd_obj->setPerspectiveName(ostr.str());
		osd_obj->addCommand("SHOW perspective");
	}
	if (system("cp -af /var/tmp/tmpcramfs/* /var/tmp/newcramfs") != 0)
	{
		osd_obj->setPerspectiveName("Error copying files.");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
		return;
	}

	osd_obj->setPerspectiveName("Downloading updates... Please wait!");
	osd_obj->addCommand("SHOW perspective");
	if (type == UPDATE_INET)
	{
		if (getUpdate() == -1)
		{
			osd_obj->setPerspectiveName("Error downloading. Check network-settings!");
			osd_obj->addCommand("SHOW perspective");
			sleep(5);
			osd_obj->addCommand("HIDE perspective");
			return;
		}
		{
			FILE *fp;
			fp = fopen("/var/tmp/newcramfs/update/version.html", "r");
			if (fp == NULL)
			{
				osd_obj->setPerspectiveName("Error version.html.");
				osd_obj->addCommand("SHOW perspective");
				sleep(5);
				osd_obj->addCommand("HIDE perspective");
				return;
			}
			char version[10];
			fgets(version, 5, fp);
			fclose(fp);
			std::string version_string(version);
			//std::cout << "Version eigen: " << settings_obj->getSmallVersion() << "-" << std::endl;
			//std::cout << "Version Server: " << version_string << "-" << std::endl;
			if (version_string == settings_obj->getSmallVersion())
			{
				osd_obj->setPerspectiveName("No update available.");
				osd_obj->addCommand("SHOW perspective");
				sleep(5);
				osd_obj->addCommand("HIDE perspective");
				return;
			}
			if (system("tar -xvzf /var/tmp/newcramfs/update/update.tar.gz -C /var/tmp/newcramfs") != 0)
			{
				osd_obj->setPerspectiveName("Error unpacking update.");
				osd_obj->addCommand("SHOW perspective");
				sleep(5);
				osd_obj->addCommand("HIDE perspective");
				return;
			}
			if (system("rm -rf /var/tmp/newcramfs/update") != 0)
			{
				osd_obj->setPerspectiveName("Error removing update-package.");
				osd_obj->addCommand("SHOW perspective");
				sleep(5);
				osd_obj->addCommand("HIDE perspective");
				return;
			}
		}
	}



	sourcedir = "/var/tmp/newcramfs";
	system("killall ftpd");
	if (system("/root/ftp/sbin/ftpd")  != 0)
	{
		osd_obj->setPerspectiveName("Error starting ftpd.");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
		return;
	}
	if (type == UPDATE_MANUALFILES || type == UPDATE_UCODES || type == UPDATE_INET)
	{
		if (type == UPDATE_MANUALFILES || type == UPDATE_INET)
		{
			osd_obj->setPerspectiveName("Please copy files and press OK!");
		}
		else if (type == UPDATE_UCODES)
		{
			osd_obj->setPerspectiveName("Please copy ucodes now and press OK!");
		}
		osd_obj->addCommand("SHOW perspective");
		int key;
		do
		{
			key = rc_obj->read_from_rc();
			if (key == RC_HOME)
			{
				osd_obj->addCommand("HIDE perspective");
				system("killall ftpd");
				return;
			}
		} while (key != RC_OK);
	}
	system("killall ftpd");
	if (type == UPDATE_MANUALFILES)
	{
		system("tar -xzvf /var/tmp/newcramfs/update/update.tar.gz -C /var/tmp/newcramfs");
		system("rm -rf /var/tmp/newcramfs/update");

	}
	osd_obj->setPerspectiveName("Making CramFS... stay tuned");
	osd_obj->addCommand("SHOW perspective");
	system("mkcramfs /var/tmp/newcramfs /var/tmp/cramfs.img");
	//mkcramfs();
	if (system("umount /var/tmp/tmpcramfs") != 0)
	{
		osd_obj->setPerspectiveName("Error unmounting tmpcramfs.");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
		return;
	}
	system("cat --help");

	eraseall();
	osd_obj->setPerspectiveName("Copying CramFS to flash... Stay tuned");
	osd_obj->addCommand("SHOW perspective");
	{
		std::stringstream ostr;
		ostr << "cat /var/tmp/cramfs.img > /dev/mtdblock/" << cramfsmtd << std::ends;
		system (ostr.str().c_str());
	}
	osd_obj->setPerspectiveName("Update Done... Reboot now!");
	osd_obj->addCommand("SHOW perspective");
	sleep(10);
	osd_obj->addCommand("HIDE perspective");
}

size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}


int update::getUpdate()
{
	CURLcode res;
	CURL *curl = curl_easy_init();

	std::string version = settings_obj->getSmallVersion();

	std::string url = "http://lcarsupdate.berlios.de/update/" + version + "/version.html";
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
	FILE *bodyfile = fopen("/var/tmp/newcramfs/update/version.html","w");
	if (bodyfile == NULL)
	{
		curl_easy_cleanup(curl);
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_FILE, bodyfile);

	res = curl_easy_perform(curl);
	fclose(bodyfile);

	bodyfile = fopen("/var/tmp/newcramfs/update/update.tar.gz","w");
	if (bodyfile == NULL)
	{
		curl_easy_cleanup(curl);
		return -1;
	}
	url = "http://lcarsupdate.berlios.de/update/" + version + "/update.tar.gz";
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	res = curl_easy_perform(curl);

	fclose(bodyfile);
	curl_easy_cleanup(curl);
	return 0;
}
