#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <enigma_streamer.h>
#include <enigma_processutils.h>

eStreamer *eStreamer::instance = 0;

eStreamer::eStreamer()
{
	if (!instance)
		instance = this;
}

eStreamer::~eStreamer()
{
}

void eStreamer::setServiceReference(eServiceReference psref)
{
	sref = psref;
}

bool eStreamer::getServiceReference(eServiceReference& psref)
{
	long *pidList = NULL;
	psref = sref;
	bool streaming;
	pidList = eProcessUtils::getPID("streamts");
	streaming = !(*pidList == -1);
	free(pidList);
	return streaming;
}
