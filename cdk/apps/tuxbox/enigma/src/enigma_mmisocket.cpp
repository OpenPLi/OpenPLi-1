#include <enigma_mmisocket.h>
#include <enigma.h>
#include <unistd.h>
#include <string.h>
#include <lib/base/eerror.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/gdi/font.h>

int eSocketMMIHandler::send_to_mmisock( void* buf, size_t len)
{
	int ret = write(connfd, buf, len);
	if ( ret < 0 )
		eDebug("[eSocketMMIHandler] write (%m)");
	else if ( (uint)ret != len )
		eDebug("[eSocketMMIHandler] only %d bytes sent.. %d bytes should be sent", ret, len );
	else
		return 0;
	return ret;
}

eSocketMMIHandler::eSocketMMIHandler()
	:connfd(-1), connsn(0), sockname("/tmp/mmi.socket"), name(0)
{
	init_eSocketMMIHandler();
}
void eSocketMMIHandler::init_eSocketMMIHandler()
{
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	unlink(sockname);
	strcpy(servaddr.sun_path, sockname);
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		eDebug("[eSocketMMIHandler] socket (%m)");
		return;
	}

	int val = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
		eDebug("[eSocketMMIHandler] SO_REUSEADDR (%m)");
	else if ((val = fcntl(listenfd, F_GETFL)) == -1)
		eDebug("[eSocketMMIHandler] F_GETFL (%m)");
	else if (fcntl(listenfd, F_SETFL, val | O_NONBLOCK) == -1)
		eDebug("[eSocketMMIHandler] F_SETFL (%m)");
	else if (bind(listenfd, (struct sockaddr *) &servaddr, clilen) == -1)
		eDebug("[eSocketMMIHandler] bind (%m)");
	else if (listen(listenfd, 0) == -1)
		eDebug("[eSocketMMIHandler] listen (%m)");
	else {
		listensn = new eSocketNotifier( eApp, listenfd, POLLIN );
		listensn->start();
		CONNECT( listensn->activated, eSocketMMIHandler::listenDataAvail );
		CONNECT( eZapSetup::setupHook, eSocketMMIHandler::setupOpened );
		eDebug("[eSocketMMIHandler] created successfully");
		return;
	}

	close(listenfd);
	listenfd = -1;
}

void eSocketMMIHandler::setupOpened( eSetupWindow* setup, int *entrynum )
{
	if (( !connsn ) || ( !name )) // no mmi connection...
		return;
	eListBox<eListBoxEntryMenu> *list = setup->getList();
	CONNECT((
			new eListBoxEntryMenu(list,
				eString().sprintf("%s Menu", name).c_str(),
				eString().sprintf("(%d) open %s menu", ++(*entrynum), name)
				))->selected, eSocketMMIHandler::initiateMMI);
}

#define CMD_SET_NAME "\x01\x02\x03\x04"

void eSocketMMIHandler::listenDataAvail(int what)
{
	if (what & POLLIN) {
		if ( connsn ) {
			eDebug("[eSocketMMIHandler] connsn != NULL");
			return;
		}
		connfd = accept(listenfd, (struct sockaddr *) &servaddr, (socklen_t *) &clilen);
		if (connfd == -1) {
			eDebug("[eSocketMMIHandler] accept (%m)");
			return;
		}

		int val;
		if ((val = fcntl(connfd, F_GETFL)) == -1)
			eDebug("[eSocketMMIHandler] F_GETFL (%m)");
		else if (fcntl(connfd, F_SETFL, val | O_NONBLOCK) == -1)
			eDebug("[eSocketMMIHandler] F_SETFL (%m)");
		else {
			connsn = new eSocketNotifier( eApp, connfd, POLLIN|POLLHUP|POLLERR );
			CONNECT( connsn->activated, eSocketMMIHandler::connDataAvail );
			return;
		}

		close(connfd);
		connfd = -1;
	}
}

void eSocketMMIHandler::connDataAvail(int what)
{
	if (what & (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND)) {
		char msgbuffer[4096];
		ssize_t length = read(connfd, msgbuffer, sizeof(msgbuffer));

		if (length == -1) {
			if (errno != EAGAIN) {
				eDebug("[eSocketMMIHandler] read (%m)");
				what |= POLLERR;
			}
		} else if (length == 0){
			what |= POLLHUP;
		} else if ((!name) && (length > 4) && (!memcmp(msgbuffer, CMD_SET_NAME, 4))) {
			length -= 4;
			name = new char[length + 1];
			memcpy(name, &msgbuffer[4], length);
			name[length] = '\0';
		} else {
			if ( !eSocketMMI::getInstance(this)->connected() ) {
				if ( eZapMain::getInstance()->isVisible() )
					eZapMain::getInstance()->hide();
			}
			eSocketMMI::getInstance(this)->gotMMIData(msgbuffer, length);
		}
	}
	
	if (what & (POLLERR | POLLHUP)) {
		closeConn();
		eSocketMMI::getInstance(this)->gotMMIData("\x9f\x88\x00\x00",4);
	}
}

void eSocketMMIHandler::closeConn()
{
	if ( connfd != -1 )
	{
		close(connfd);
		connfd=-1;
	}
	if ( connsn )
	{
		delete connsn;
		connsn=0;
	}
	if ( name )
	{
		delete [] name;
		name=0;
	}
}

eSocketMMIHandler::~eSocketMMIHandler()
{
	closeConn();
	delete listensn;
	unlink(sockname);
}

void eSocketMMIHandler::initiateMMI()
{
	int ret=0;
	{
		unsigned char buf[]="\x9F\x80\x22";  // ENTER MENU
		ret = send_to_mmisock(buf, 3);
	}
	if ( ret )
		return;
	eWindow *setup = (eWindow*)currentFocus;
	setup->hide();
	eSocketMMI::getInstance(this)->exec();
	{
		unsigned char buf[]="\x9F\x88\x00";  // CLOSE MMI
		send_to_mmisock(buf, 3);
	}
	setup->show();
}

extern long LengthField(unsigned char *lengthfield,long maxlength,int *fieldlen);

std::map<eSocketMMIHandler*,eSocketMMI*> eSocketMMI::exist;

eSocketMMI* eSocketMMI::getInstance( eSocketMMIHandler *handler  )
{
	std::map<eSocketMMIHandler*, eSocketMMI*>::iterator it = exist.find(handler);
	if ( it == exist.end() )
		exist[handler]=new eSocketMMI(handler);
	return exist[handler];
}

eSocketMMI::eSocketMMI( eSocketMMIHandler *handler )
	:handler(handler)
{
	init_eSocketMMI();
}
void eSocketMMI::init_eSocketMMI()
{
	setText(eString().sprintf("%s - mmi", handler->getName()));
	lText->setText(eString().sprintf("waiting for %s answer...", handler->getName()));
	int newHeight = size.height() - getClientSize().height() + lText->getExtend().height() + 10 + 20;
	resize( eSize( size.width(), newHeight ) );
}

void eSocketMMI::sendAnswer( AnswerType ans, int param, unsigned char *data )
{
	switch(ans)
	{
		case ENQAnswer:
		{
			unsigned char buffer[ data[0]+4 ];
			memcpy(buffer,"\x9f\x88\x08", 3);
			memcpy(buffer+3, data, data[0]+1 );
			for (int i=0; i < data[0]+4; i++ )
				eDebugNoNewLine("%02x ", buffer[i]);
			eDebug("");
			handler->send_to_mmisock( buffer, data[0]+4 );
			break;
		}
		case LISTAnswer:
		case MENUAnswer:
		{
			unsigned char buffer[5];
			memcpy(buffer,"\x9f\x88\x0B\x1",4);
			buffer[4]=param&0xff;
			handler->send_to_mmisock( buffer, 5 );
			break;
		}
	}
}

void eSocketMMI::beginExec()
{
	conn = CONNECT(handler->mmi_progress, enigmaMMI::gotMMIData );
}

eAutoInitP0<eSocketMMIHandler> init_eSocketMMIHandler(eAutoInitNumbers::osd-2, "socket mmi handler");
