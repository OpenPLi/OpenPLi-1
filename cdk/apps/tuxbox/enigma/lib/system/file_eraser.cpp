#include <lib/system/file_eraser.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

eBackgroundFileEraser *eBackgroundFileEraser::instance;

eBackgroundFileEraser::eBackgroundFileEraser()
	:messages(this,1)
{
	if (!instance)
		instance=this;
	CONNECT(messages.recv_msg, eBackgroundFileEraser::gotMessage);
	run();
}

eBackgroundFileEraser::~eBackgroundFileEraser()
{
	messages.send(Message::quit);
	if ( thread_running() )
		kill();
	if (instance==this)
		instance=0;
}

void eBackgroundFileEraser::thread()
{
	nice(5);
	exec();
}

void eBackgroundFileEraser::erase(const char *filename)
{
	messages.send(Message(Message::erase, filename?strdup(filename):0));
}

void eBackgroundFileEraser::gotMessage(const Message &msg )
{
	switch (msg.type)
	{
		case Message::erase:
			if ( msg.filename )
			{
				if ( ::unlink(msg.filename) < 0 )
					eDebug("remove file %s failed (%m)", msg.filename);
				else
					eDebug("file %s erased", msg.filename);
				free((char*)msg.filename);
			}
			break;
		case Message::quit:
			quit(0);
			break;
		default:
			eDebug("unhandled thread message");
	}
}

eAutoInitP0<eBackgroundFileEraser> init_eBackgroundFilEraser(eAutoInitNumbers::configuration+1, "Background File Eraser");
