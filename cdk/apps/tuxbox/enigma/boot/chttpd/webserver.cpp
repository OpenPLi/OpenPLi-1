/*
 * $Id: webserver.cpp,v 1.4 2005/10/18 11:30:19 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 * based on nhttpd (C) 2001/2002 Dirk Szymanski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef ENABLE_EXPERT_WEBIF
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <sys/wait.h> 

#include "webserver.h"
#include "request.h"
#include "debug.h"
#include <chttpd/chttpdconfig.h>

extern chttpdConfig cfg;

struct Cmyconn
{
	socklen_t clilen;
	string Client_Addr;
	int Socket;
	CWebserver *Parent;	
};

pthread_mutex_t ServerData_mutex;
unsigned long Requests = 0;
int ThreadsCount = 0;

CWebserver::CWebserver(bool debug)
{
	ListenSocket = 0;
	STOP = false;
 
	cfg.load();
}

CWebserver::~CWebserver()
{

	if (ListenSocket)
		Stop();
}

bool CWebserver::Init(bool debug)
{
	return true;
}

bool CWebserver::Start()
{
	SAI servaddr;

	//network-setup
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(cfg.Port);
	
	if ( bind(ListenSocket, (SA *) &servaddr, sizeof(servaddr)) != 0)
	{
		int i = 1;
		do
		{
			aprintf("bind to port %d failed...\n", cfg.Port);
			aprintf("%d. Versuch, warte 5 Sekunden\n",i++);
			sleep(5);
		} while (bind(ListenSocket, (SA *) &servaddr, sizeof(servaddr)) != 0);
//		return false;
	}

	if (listen(ListenSocket, 5) != 0)
	{
			perror("listen failed...");
			return false;
	}
	dprintf("Server gestartet\n");
	
	return true;
}

void * WebThread(void * myconn)
{
	CWebserverRequest *req;
	pthread_detach(pthread_self());
	req = new CWebserverRequest(((Cmyconn *)myconn)->Parent);
	req->Client_Addr = ((Cmyconn *)myconn)->Client_Addr;
	req->Socket = ((Cmyconn *)myconn)->Socket;
	
	pthread_mutex_lock( &ServerData_mutex );
	req->RequestNumber = Requests++;
	ThreadsCount++;
	pthread_mutex_unlock( &ServerData_mutex );
	if(req->GetRawRequest())
	{
		while (ThreadsCount > 15)
		{
			aprintf("Too many requests, waitin one sec\n");
			sleep(1);
		}
		dprintf("++ Thread 0x06%X gestartet, ThreadCount: %d\n",(int)pthread_self(),ThreadsCount);	
		if(req->ParseRequest())
		{
			req->SendResponse();
			req->PrintRequest();
			req->EndRequest();
		}
		else
			dprintf("Error while parsing request\n");

		pthread_mutex_lock( &ServerData_mutex );
		ThreadsCount--;
		pthread_mutex_unlock( &ServerData_mutex );

		dprintf("-- Thread 0x06%X beendet, ThreadCount: %d\n",(int)pthread_self(),ThreadsCount);
		delete req;
		delete (Cmyconn *) myconn;
	}
	pthread_exit((void *)NULL);
	return NULL;
}

void CWebserver::DoLoop()
{
	socklen_t clilen;
	SAI cliaddr;
	CWebserverRequest *req;
	int sock_connect;
	int thread_num =0;
	int t = 1;
	pthread_t Threads[30];
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init( &ServerData_mutex, NULL );

	clilen = sizeof(cliaddr);
	while(!STOP)
	{
		memset(&cliaddr, 0, sizeof(cliaddr));
		if ((sock_connect = accept(ListenSocket, (SA *) &cliaddr, &clilen)) == -1)	// accepting requests
		{
			perror("Error in accept");
			continue;
		}
		setsockopt(sock_connect,SOL_TCP,TCP_CORK,&t,sizeof(t));
		dprintf("chttpd: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));	// request from client arrives

		if(cfg.THREADS)		
		{						// Multi Threaded 
			Cmyconn *myconn = new Cmyconn;		// prepare Cmyconn struct
			myconn->clilen = clilen;
			myconn->Client_Addr = inet_ntoa(cliaddr.sin_addr);
			myconn->Socket = sock_connect;
			myconn->Parent = this;
			if (pthread_create (&Threads[thread_num], &attr, WebThread, (void *)myconn) != 0 ) // start WebThread 
				dperror("pthread_create(WebThread)");
			if (thread_num == 20)			// testing
				thread_num = 0;
			else
				thread_num++;
		}
		else
		{						// Single Threaded
			req = new CWebserverRequest(this);	// create new request
			req->Socket = sock_connect;	
			req->Client_Addr = inet_ntoa(cliaddr.sin_addr);
			req->RequestNumber = Requests++;
//			if(DEBUG) printf("GetRawRequest()\n");
			if(req->GetRawRequest())		//read request from client
			{					//if(DEBUG) printf("ParseRequest()\n");
				if(req->ParseRequest())		// parse it
				{
//					if(DEBUG) printf("SendResponse()\n");				
					req->SendResponse();	// send the proper response
					req->PrintRequest();	// and print if wanted
				}
				else
					dperror("Error while parsing request");
//				if(DEBUG) printf("EndRequest()\n");
				req->EndRequest();		// end the request
				delete req;
				req = NULL;
			}
			else
				dperror("Unable to read request");
		}
	}
}

void CWebserver::Stop()
{
	if (ListenSocket != 0)
	{
//		if(DEBUG) printf("ListenSocket closed\n");
		close( ListenSocket );					
		ListenSocket = 0;
	}
}

int CWebserver::SocketConnect(Tmconnect * con, int Port)
{
	char rip[]="127.0.0.1";

	con->sock_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&con->servaddr,0,sizeof(SAI));
	con->servaddr.sin_family=AF_INET;
	con->servaddr.sin_port=htons(Port);
	inet_pton(AF_INET, rip, &con->servaddr.sin_addr);

	#ifdef HAS_SIN_LEN
		servaddr.sin_len = sizeof(servaddr); // needed ???
	#endif


	if (connect(con->sock_fd, (SA *)&con->servaddr, sizeof(con->servaddr))==-1)
	{
		aprintf("[chttp]: connect to socket %d failed\n",Port);
		return -1;
	}
	else
		return con->sock_fd;
}
#endif

