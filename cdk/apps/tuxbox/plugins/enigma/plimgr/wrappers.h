#ifndef _wrappers_h
#define _wrappers_h

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

ssize_t Write(int fd, const void *buf, size_t count);
ssize_t Read(int fd, void *buf, size_t count);

int Select(int iMaxfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
int Bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
int Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#endif
