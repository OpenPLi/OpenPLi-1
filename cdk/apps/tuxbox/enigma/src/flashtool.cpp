#ifdef ENABLE_FLASHTOOL
/**********************************************
*
*	$Revision: 1.13 $
*
**********************************************/

#include <flashtool.h>
#include <enigma.h>

#include <lib/gui/emessage.h>
#include <lib/gui/eprogress.h>
#include <lib/dvb/decoder.h>

#include <sys/wait.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#include <linux/compiler.h>
#endif
#include <mtd/mtd-user.h>

class FlashProgressWindow: public eWindow
{
	void init_FlashProgressWindow(const char* wtext);
public:
	eProgress progress;
	FlashProgressWindow(const char *);
};

FlashProgressWindow::FlashProgressWindow(const char *wtext)
	:eWindow(0), progress(this)
{
	init_FlashProgressWindow(wtext);
}
void FlashProgressWindow::init_FlashProgressWindow(const char* wtext)
{
	cresize(eSize(470,50));
	valign();
	setText(wtext);
	progress.move(ePoint(10,10));
	progress.resize(eSize(450,30));
}


eFlashtoolMain::eFlashtoolMain()
	:eListBoxWindow<eListBoxEntryText>(_("Flashtool"), 2, 350)
{
	init_eFlashtoolMain();
}
void eFlashtoolMain::init_eFlashtoolMain()
{
	valign();
	new eListBoxEntryText(&list, _("Save image or part of it"), (void *)0);
	new eListBoxEntryText(&list, _("Flash image or part of it"), (void *)1);
	CONNECT(list.selected, eFlashtoolMain::sel_item);
}

void eFlashtoolMain::sel_item(eListBoxEntryText *sel)
{
	if (sel)
	{
		int flashtool=(int)sel->getKey();
		/*if (flashtool==-1)
		{
			close(0);
			return;
		}*/

		hide();
		eFlashtool flash(flashtool);
		flash.show();
		flash.exec();
		flash.hide();
		show();
	}
}

eFlashtoolMain::~eFlashtoolMain()
{
}

eFlashtool::eFlashtool(int direction)
	:eListBoxWindow<eListBoxEntryText>(_("Partitions"), 12, 400)
{
	/* direction == 0   -->  read
	 * direction == 1   -->  write
	 */

	flashimage = direction;
	valign();
	//new eListBoxEntryText(&list, _("back"), (void *)-1);

	FILE* fd = fopen("/proc/mtd", "r");
	if (!fd)
	{
		perror("cannot read /proc/mtd");
		return;
	}

	int entries = 0;
	char buf[1000];
	fgets(buf,sizeof(buf), fd);
	while(!feof(fd))
	{
		if (fgets(buf, sizeof(buf), fd) != NULL)
		{
			char mtdname[50] = "";
			int mtdnr = 0;
			int mtdsize = 0;
			int mtderasesize = 0;
			sscanf(buf, "mtd%d: %x %x \"%[a-zA-Z0-9 =/+`äö<>@^°!?'.:,_-()#\\]\"\n", &mtdnr, &mtdsize, &mtderasesize, mtdname);
			new eListBoxEntryText(&list, _(mtdname), (void*)mtdnr);
			entries++;
		}
	}
	fclose(fd);

	cresize(eSize(400, 10 + entries * eListBoxEntryText::getEntryHeight()));
	list.move(ePoint(10, 5));
	list.resize(eSize(getClientSize().width() - 20, getClientSize().height() - 5));

	CONNECT(list.selected, eFlashtool::sel_item);
}

eFlashtool::~eFlashtool()
{
}

void eFlashtool::sel_item(eListBoxEntryText *sel)
{
	if (sel)
	{
		mtd = (int)sel->getKey();

		/*if (mtd == -1)
		{
			eWidget::accept();
			return;
		}*/

		hide();

		char destination[100];
		eFlashtoolSource imagesource(flashimage);
		imagesource.show();
		int res = imagesource.exec();
		imagesource.hide();
		strcpy(destination, imagesource.getDestination());

		if (res == 2)
			if (flashimage == 0)
				readmtd(destination);
			else
			{
				int res = eMessageBox::ShowBox(_("Do you really want to flash a new image now?"), _("Flash"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconWarning, eMessageBox::btNo);
				if (res == eMessageBox::btYes)
					programm(destination);
			}
		show();
	}
}

void eFlashtool::programm(char filename[])
{
	char mtddev[20];

	sprintf(mtddev,"/dev/mtd/%d", mtd);

	if (access(filename, R_OK) == 0)
	{
		int fd1, fd2;
		unsigned long filesize;

		hide();

		if ((fd1 = open(filename, O_RDONLY)) < 0)
		{
			eMessageBox::ShowBox(_("Can't read image!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);
			return;
		}

		filesize = lseek(fd1, 0, SEEK_END);
		lseek(fd1, 0, SEEK_SET);

		if (filesize==0)
		{
			eMessageBox::ShowBox(_("Image has file size of 0 bytes!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);
			return;
		}

		if ((fd2 = open(mtddev, O_WRONLY)) < 0)
		{
			eMessageBox::ShowBox(_("Can't open mtd!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);
			::close(fd2);
			return;
		}

		mtd_info_t meminfo;
		if (ioctl(fd2, MEMGETINFO, &meminfo) != 0)
		{
			return;
		}

		if (filesize > meminfo.size)
		{
			eMessageBox::ShowBox(_("Image size is too big! Can't flash!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);
			return;
		}

		if (filesize < ((meminfo.size/100)*70))
		{
			int res = eMessageBox::ShowBox(_("Image size is too small! Do you really want to flash it?"), _("Flash"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconWarning, eMessageBox::btNo);
			if (res == eMessageBox::btNo)
			{
				return;
			}
		}

		eMessageBox mb(_("Please wait... do NOT switch off the receiver!"),_("flashing in progress"), eMessageBox::iconWarning);
		mb.show();

		sync();
		Decoder::Flush();

		// without this nice we have not enough priority for
		// file operations... then the update ist very slow on the
		// dreambox
		nice(-10);

		printf("Starting erasing %s...\n",mtddev);

		if (!erase())
		{
			eMessageBox::ShowBox(_("Erase error!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);
			return;
		}

		FlashProgressWindow wnd(_("Writing image to flash...\n"));
		wnd.show();

		char buf[meminfo.erasesize];
		long fsize = filesize;

		printf("flashing now...\n");
		while (fsize > 0)
		{
			long block = fsize;
			if (block>(long)sizeof(buf))
			{
				block = sizeof(buf);
			}
			read(fd1, buf, block);
			write(fd2, buf, block);
			fsize -= block;
			wnd.progress.setPerc(((filesize - fsize) * 100) / filesize);
		}

		::close(fd1);
		::close(fd2);
		wnd.hide();
		mb.hide();
		printf("finished\n");
		eMessageBox::ShowBox(_("Flashing successful! restarting..."),
				_("flashing ok"),eMessageBox::btOK|eMessageBox::iconInfo);
		::reboot(RB_AUTOBOOT);
		system("reboot");
		return;
	}
	else
	{
		eMessageBox::ShowBox(_("Image not found!"), _("Flash"), eMessageBox::iconWarning|eMessageBox::btOK);
	}
}

bool eFlashtool::erase()
{
	int fd;
	mtd_info_t meminfo;
	erase_info_t erase;

	eString mtddev;
	mtddev.sprintf("/dev/mtd/%d", mtd);

	if ((fd = open(mtddev.c_str(), O_RDWR)) < 0)
	{
		return false;
	}

	if (ioctl(fd, MEMGETINFO, &meminfo) != 0)
	{
		return false;
	}

	FlashProgressWindow wnd(_("Erasing flash..."));
	wnd.show();
	erase.length = meminfo.erasesize;
	for (erase.start = 0; erase.start < meminfo.size; erase.start += meminfo.erasesize)
	{

		printf("\rErasing %u KByte @ %x -- %2u %% complete.",
		                 meminfo.erasesize / 1024, erase.start,
		                 erase.start * 100 / meminfo.size);

		wnd.progress.setPerc(erase.start * 100 / meminfo.size);
		if (ioctl(fd, MEMERASE, &erase) != 0)
		{
			::close(fd);
			return false;
		}
	}

	::close(fd);
	wnd.hide();
	return true;
}

bool eFlashtool::readmtd(char destination[])
{
	int fd1, fd2;
	char mtddev[50];
	long filesize;
	mtd_info_t meminfo;

	FILE *fd = fopen("/.version", "r");

	char filename[50];
	sprintf(filename, "%s/mtd%d.img", destination, mtd);
	if (fd)
	{
		char buffer[100];
		fgets(buffer, sizeof(buffer), fd);
		fclose(fd);
		char date[9];
		char time[5];
		sscanf(buffer,"version=%*2s%*2s%8s%4s", date, time);
		sprintf(filename,"%s/%s_%s_mtd%d.img", destination, date, time, mtd);
	}

	sprintf(mtddev, "/dev/mtd/%d", mtd);

	if ((fd1 = open(mtddev, O_RDONLY)) < 0)
	{
		eMessageBox::ShowBox(_("Can't open mtd device!"), _("Flash"), eMessageBox::iconWarning|eMessageBox::btOK);
		return false;
	}

	if (ioctl(fd1, MEMGETINFO, &meminfo) != 0)
	{
		return false;
	}

	if ((fd2 = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR  |  S_IRGRP | S_IWGRP  |  S_IROTH | S_IWOTH)) < 0)
	{
		eString message = eString(_("Can't open ")) + eString(destination) + eString(_(" for writing!"));
		eMessageBox::ShowBox(message.c_str(), _("Flash"), eMessageBox::iconWarning|eMessageBox::btOK);
		::close(fd1);
		return false;
	}

	filesize = meminfo.size;

	eMessageBox mb(_("Please wait while partition is being saved..."),_("saving in progress"), eMessageBox::iconInfo);
	mb.show();

	FlashProgressWindow wnd(_("Read Flash..."));
	wnd.show();
	char buf[meminfo.erasesize];
	long fsize = filesize;
	while(fsize > 0)
	{
		long block = fsize;
		if (block > (long)sizeof(buf))
			block = sizeof(buf);
		read(fd1, &buf, block);
		write(fd2, &buf, block);
		fsize -= block;
		wnd.progress.setPerc(((filesize - fsize) * 100) / filesize);
	}
	::close(fd1);
	::close(fd2);
	wnd.hide();

	eString message = eString(_("Image saved to: ")) + eString(filename);
	eMessageBox::ShowBox(message.c_str(), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK);

	mb.hide();

	return true;
}

eFlashtoolImageView::eFlashtoolImageView(char folder[])
	:eListBoxWindow<eListBoxEntryText>(_("Image list"), 11, 530)
{
	new eListBoxEntryText(&list, _("back"), (void *)new eString("back"), 0, "Choose your image and press OK...");

	valign();
	int entries = 2;
	struct dirent **namelist;
	int n = scandir(folder, &namelist, 0, alphasort);
	if (n < 0)
	{
		perror("no flash images available");
	}
	else
	{
		for(int count = 0; count < n; count++)
		{
			eString filen = namelist[count]->d_name;
			int pos = filen.find(".img");
			if (pos != -1)
			{
				char tmp[100];
				char helptext[100];
				int fd1;
				sprintf(tmp, "%s/%s", folder, filen.c_str());
				if ((fd1 = open(tmp, O_RDONLY)) < 0)
					sprintf(helptext, "WARNING: can't open this image!");
				else
				{
					int filesize = lseek(fd1, 0, SEEK_END);
					lseek(fd1, 0, SEEK_SET);
					sprintf(helptext, "Image size: %d byte\n", filesize);
					::close(fd1);
				}

				new eListBoxEntryText(&list, filen, (void *)new eString(tmp), 0, helptext);
				entries++;
			}
			free(namelist[count]);
		}
		free(namelist);
	}

	cresize(eSize(530, 10 + (entries > 10 ? 10 : entries) * eListBoxEntryText::getEntryHeight()));
	list.move(ePoint(10, 5));
	list.resize(eSize(getClientSize().width() - 20, getClientSize().height() - 30));

	CONNECT(list.selected, eFlashtoolImageView::sel_item);

	statusbar = new eStatusBar(this);
	statusbar->move(ePoint(0, clientrect.height() - 30));
	statusbar->resize(eSize(clientrect.width(), 30));
	statusbar->setText("");
	statusbar->loadDeco();
}

void eFlashtoolImageView::sel_item(eListBoxEntryText *sel)
{
	if (sel)
	{
		sprintf(buffer,((eString *)sel->getKey())->c_str());
		close(1);
	}
	else
		close(0);
}

char* eFlashtoolImageView::getFilename()
{
	return buffer;
}

eFlashtoolImageView::~eFlashtoolImageView()
{
}

eFlashtoolSource::eFlashtoolSource(int direction)
#ifndef DISABLE_FILE
	:eListBoxWindow<eListBoxEntryText>(_("Save to..."), 5, 400)
#else
	:eListBoxWindow<eListBoxEntryText>(_("Save to..."), 3, 400)
#endif
{
	flash = false;

	if (direction == 1)
	{
		setText(_("Select source..."));
		flash = true;
	}

	valign();
	//new eListBoxEntryText(&list, _("back"), (void *)"back");
	new eListBoxEntryText(&list, "tmp", (void *)"/tmp");
#ifndef DISABLE_FILE
	if (access("/media/hdd/images", W_OK) == 0)
		new eListBoxEntryText(&list, "HDD", (void *)"/media/hdd/images");
	if (access("/media/usb/images", W_OK) == 0)
		new eListBoxEntryText(&list, "USB", (void *)"/media/usb/images");
#endif
	if (access("/var/mnt/nfs/images", W_OK) == 0)
		new eListBoxEntryText(&list, "NFS", (void *)"/var/mnt/nfs/images");
	if (access("/var/mnt/cifs/images", W_OK) == 0)
		new eListBoxEntryText(&list, "CIFS", (void *)"/var/mnt/cifs/images");
	CONNECT(list.selected, eFlashtoolSource::sel_item);
}

void eFlashtoolSource::sel_item(eListBoxEntryText *sel)
{
	if (sel)
	{
		strcpy(buffer, (char *)sel->getKey());

		if (strcmp(buffer, "back") == 0)
		{
			close(1);
			return;
		}

		hide();

		int res = 1;
		if (flash)
		{
			eFlashtoolImageView image(buffer);
			image.show();
			res = image.exec();
			image.hide();
			strcpy(buffer, image.getFilename());
		}
		if (strcmp(buffer, "back") == 0 || res == 0)
			show();
		else
			close(2);
	}
}

char* eFlashtoolSource::getDestination()
{
	return buffer;
}

eFlashtoolSource::~eFlashtoolSource()
{
}

#endif // ENABLE_FLASHTOOL
