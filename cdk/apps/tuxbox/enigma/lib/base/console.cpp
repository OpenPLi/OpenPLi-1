/*
 * console.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: console.cpp,v 1.17 2009/02/03 18:52:52 dbluelle Exp $
 */

#include <lib/base/console.h>

#include <lib/base/estring.h>
#include <sys/vfs.h> // for statfs
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int bidirpipe(int pfd[], char *cmd , char *argv[])
{
	int pfdin[2];  /* from child to parent */
	int pfdout[2]; /* from parent to child */
	int pfderr[2]; /* stderr from child to parent */
	int pid;       /* child's pid */

	if ( pipe(pfdin) == -1 || pipe(pfdout) == -1 || pipe(pfderr) == -1)
		return(-1);

	if ( ( pid = fork() ) == -1 )
		return(-1);
	else if (pid == 0) /* child process */
	{
		setsid();
		if ( close(0) == -1 || close(1) == -1 || close(2) == -1 )
			_exit(0);

		if (dup(pfdout[0]) != 0 || dup(pfdin[1]) != 1 || dup(pfderr[1]) != 2 )
			_exit(0);

		if (close(pfdout[0]) == -1 || close(pfdout[1]) == -1 ||
				close(pfdin[0]) == -1 || close(pfdin[1]) == -1 ||
				close(pfderr[0]) == -1 || close(pfderr[1]) == -1 )
			_exit(0);

		for (unsigned int i=3; i < 90; ++i )
			close(i);

		execvp(cmd,argv);
		_exit(0);
	}
	if (close(pfdout[0]) == -1 || close(pfdin[1]) == -1 || close(pfderr[1]) == -1)
			return(-1);

	pfd[0] = pfdin[0];
	pfd[1] = pfdout[1];
	pfd[2] = pfderr[0];

	return(pid);
}

eConsoleAppContainer::eConsoleAppContainer( const eString &cmd )
:pid(-1), killstate(0)
{
	init_eConsoleAppContainer(cmd);
}
void eConsoleAppContainer::init_eConsoleAppContainer( const eString &cmd )
{
//	eDebug("cmd = %s", cmd.c_str() );
	for (int i=0; i < 3; ++i)
		fd[i]=-1;
	int cnt=2; // path to app + terminated 0
	eString str(cmd?cmd:"");

	while( str.length() && str[0] == ' ' )  // kill spaces at beginning
		str = str.mid(1);

	while( str.length() && str[str.length()-1] == ' ' )  // kill spaces at the end
		str = str.left( str.length() - 1 );

	if (!str.length())
		return;

	std::map<char,char> brackets;
	brackets.insert(std::pair<char,char>('\'','\''));
	brackets.insert(std::pair<char,char>('"','"'));
	brackets.insert(std::pair<char,char>('`','`'));
	brackets.insert(std::pair<char,char>('(',')'));
	brackets.insert(std::pair<char,char>('{','}'));
	brackets.insert(std::pair<char,char>('[',']'));
	brackets.insert(std::pair<char,char>('<','>'));

	unsigned int idx=0;
	eString path = str.left( (idx = str.find(' ')) != eString::npos ? idx : str.length() );
//	eDebug("path = %s", path.c_str() );

	eString cmds = str.mid( path.length()+1 );
//	eDebug("cmds = %s", cmds.c_str() );

	idx = 0;
	std::map<char,char>::iterator it = brackets.find(cmds[idx]);
	while ( (idx = cmds.find(' ',idx) ) != eString::npos )  // count args
	{
		if (it != brackets.end())
		{
			if (cmds[idx-1] == it->second)
				it = brackets.end();
		}
		if (it == brackets.end())
		{
			cnt++;
			it = brackets.find(cmds[idx+1]);
		}
		idx++;
	}

//	eDebug("idx = %d, %d counted spaces", idx, cnt-2);

	if ( cmds.length() )
	{
		cnt++;
//		eDebug("increase cnt");
	}

//	eDebug("%d args", cnt-2);
	char **argv = new char*[cnt];  // min two args... path and terminating 0
	argv[0] = new char[ path.length() ];
	strcpy( argv[0], path.c_str() );
	argv[cnt-1] = 0;               // set terminating null

	if ( cnt > 2 )  // more then default args?
	{
		cnt=1;  // do not overwrite path in argv[0]

		it = brackets.find(cmds[0]);
		idx=0;
		while ( (idx = cmds.find(' ',idx)) != eString::npos )  // parse all args..
		{
			bool bracketClosed=false;
			if ( it != brackets.end() )
			{
				if (cmds[idx-1]==it->second)
				{
					it = brackets.end();
					bracketClosed=true;
				}
			}
			if ( it == brackets.end() )
			{
				eString tmp = cmds.left(idx);
				if (bracketClosed)
				{
					tmp.erase(0,1);
					tmp.erase(tmp.length()-1, 1);
					bracketClosed=false;
				}
				argv[cnt] = new char[ tmp.length()+1 ];
//				eDebug("idx=%d, arg = %s", idx, tmp.c_str() );
				strcpy( argv[cnt++], tmp.c_str() );
				cmds = cmds.mid(idx+1);
//				eDebug("str = %s", cmds.c_str() );
				it = brackets.find(cmds[0]);
				idx=0;
			}
			else
				idx++;
		}
		if ( it != brackets.end() )
		{
			cmds.erase(0,1);
			cmds.erase(cmds.length()-1, 1);
		}
		// store the last arg
		argv[cnt] = new char[ cmds.length() ];
		strcpy( argv[cnt], cmds.c_str() );
	}
	else
		cnt=1;

  // get one read ,one write and the err pipe to the prog..

//	int tmp=0;
//	while(argv[tmp])
//		eDebug("%d is %s", tmp, argv[tmp++]);
  
	pid = bidirpipe(fd, argv[0], argv);

	while ( cnt >= 0 )  // release heap memory
		delete [] argv[cnt--];
	delete [] argv;
	
	if ( pid == -1 )
		return;

//	eDebug("pipe in = %d, out = %d, err = %d", fd[0], fd[1], fd[2]);

	in = new eSocketNotifier(eApp, fd[0], 19 );  // 19 = POLLIN, POLLPRI, POLLHUP
	out = new eSocketNotifier(eApp, fd[1], eSocketNotifier::Write, false);  // POLLOUT
	err = new eSocketNotifier(eApp, fd[2], 19 );  // 19 = POLLIN, POLLPRI, POLLHUP
	CONNECT(in->activated, eConsoleAppContainer::readyRead);
	CONNECT(out->activated, eConsoleAppContainer::readyWrite);
	CONNECT(err->activated, eConsoleAppContainer::readyErrRead);
}

eConsoleAppContainer::~eConsoleAppContainer()
{
	kill();
	delete in;
	delete out;
	delete err;
}

void eConsoleAppContainer::kill()
{
	if ( killstate != -1 )
	{
		eDebug("user kill(SIGKILL) console App");
		killstate=-1;
		/*
		 * Use a negative pid value, to signal the whole process group
		 * ('pid' might not even be running anymore at this point)
		 */
		::kill(-pid, SIGKILL);
		closePipes();
	}
}

void eConsoleAppContainer::sendCtrlC()
{
	if ( killstate != -1 )
	{
		eDebug("user send SIGINT(Ctrl-C) to console App");
		/*
		 * Use a negative pid value, to signal the whole process group
		 * ('pid' might not even be running anymore at this point)
		 */
		::kill(-pid, SIGINT);
	}
}


void eConsoleAppContainer::closePipes()
{
	in->stop();
	out->stop();
	err->stop();
	::close(fd[0]);
	fd[0]=-1;
	::close(fd[1]);
	fd[1]=-1;
	::close(fd[2]);
	fd[2]=-1;
	eDebug("pipes closed");
}

void eConsoleAppContainer::readyRead(int what)
{
	if (what & POLLPRI|POLLIN)
	{
//		eDebug("what = %d");
		char buf[2048];
		int readed = read(fd[0], buf, 2048);
//		eDebug("%d bytes read", readed);
		if ( readed != -1 && readed )
		{
/*			for ( int i = 0; i < readed; i++ )
				eDebug("%d = %c (%02x)", i, buf[i], buf[i] );*/
			/*emit*/ dataAvail( eString( buf, readed ) );
		}
		else if (readed == -1)
			eDebug("readerror %d", errno);
	}
	if (what & eSocketNotifier::Hungup)
	{
		eDebug("child has terminated");
		closePipes();
		int childstatus;
		int retval = killstate;
		/*
		 * We have to call 'wait' on the child process, in order to avoid zombies.
		 * Also, this gives us the chance to provide better exit status info to appClosed.
		 */
		if (::waitpid(pid, &childstatus, 0) > 0)
		{
			if (WIFEXITED(childstatus))
			{
				retval = WEXITSTATUS(childstatus);
			}
		}
		/*emit*/ appClosed(retval);
	}
}

void eConsoleAppContainer::readyErrRead(int what)
{
	if (what & POLLPRI|POLLIN)
	{
//		eDebug("what = %d");
		char buf[2048];
		int readed = read(fd[2], buf, 2048);
//		eDebug("%d bytes read", readed);
		if ( readed != -1 && readed )
		{
/*			for ( int i = 0; i < readed; i++ )
				eDebug("%d = %c (%02x)", i, buf[i], buf[i] );*/
			/*emit*/ dataAvail( eString( buf, readed ) );
		}
		else if (readed == -1)
			eDebug("readerror %d", errno);
	}
}

void eConsoleAppContainer::write( const char *data, int len )
{
	char *tmp = new char[len];
	memcpy(tmp, data, len);
	outbuf.push(queue_data(tmp,len));
	out->start();
}

void eConsoleAppContainer::readyWrite(int what)
{
	if (what == POLLOUT && outbuf.size() )
	{
		queue_data d = outbuf.front();
		outbuf.pop();
		if ( ::write( fd[1], d.data, d.len ) != d.len )
		{
			/* emit */ dataSent(-1);
//			eDebug("writeError");
		}
		else
		{
			/* emit */ dataSent(0);
//			eDebug("write ok");
		}

		delete [] d.data;
	}
	if ( !outbuf.size() )
		out->stop();
}
