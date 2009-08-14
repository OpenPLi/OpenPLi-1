#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dbox/fp.h>
#include <lib/system/dmfp.h>

int eDreamboxFP::isUpgradeAvailable()
{
	int fd = ::open("/dev/dbox/fp0", O_RDWR);
	if (fd < 0)
		return 0;
	int val = 0xdeadbabe;
	if ((::ioctl(fd, 0x200, &val) < 0) || (val != 1))
	{
		::close(fd);
		return 0;
	}
	::close(fd);
	return 1;
}

int eDreamboxFP::doUpgrade(int magic)
{
	sync();
	int fd = ::open("/dev/dbox/fp0", O_RDWR);
	if (fd < 0)
		return 0;
	int res = ::ioctl(fd, 0x200, &magic);
	::close(fd);
	return res;
}

int eDreamboxFP::getFPVersion()
{
	int fd = ::open("/dev/dbox/fp0", O_RDWR);
	if (fd < 0)
		return 0;
	int val = ::ioctl(fd, FP_IOCTL_GETID);
	::close(fd);
	return val;
}
