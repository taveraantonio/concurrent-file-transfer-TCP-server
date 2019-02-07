/*
 
 module: sockwrap.h
 
 purpose: definitions of functions in sockwrap.c
 
 reference: Stevens, Unix network programming (3ed)
 
 */


#ifndef _SOCKWRAP_H

#define _SOCKWRAP_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define SA struct sockaddr

#define INTERRUPTED_BY_SIGNAL (errno == EINTR)

typedef	void	Sigfunc(int);	/* for signal handlers */

int Socket (int family, int type, int protocol);

void Bind (int sockfd, const SA *myaddr, socklen_t myaddrlen);

void Listen (int sockfd, int backlog);

int Accept (int listen_sockfd, SA *cliaddr, socklen_t *addrlenp);

void Connect (int sockfd, const SA *srvaddr, socklen_t addrlen);

void Close (int fd);

void Shutdown (int fd, int howto);

ssize_t Read (int fd, void *bufptr, size_t nbytes);

void Write (int fd, void *bufptr, size_t nbytes);

ssize_t Recv (int fd, void *bufptr, size_t nbytes, int flags);

ssize_t Recvfrom (int fd, void *bufptr, size_t nbytes, int flags, SA *sa, socklen_t *salenptr);

void Sendto (int fd, void *bufptr, size_t nbytes, int flags, const SA *sa, socklen_t salen);

void Send (int fd, void *bufptr, size_t nbytes, int flags);

void Inet_aton (const char *strptr, struct in_addr *addrptr);

void Inet_pton (int af, const char *strptr, void *addrptr);

void Inet_ntop (int af, const void *addrptr, char *strptr, size_t length);

ssize_t readn (int fd, void *vptr, size_t n);

ssize_t Readn (int fd, void *ptr, size_t nbytes);

ssize_t readline_buffered (int fd, void *vptr, size_t maxlen);

ssize_t readline_unbuffered (int fd, void *vptr, size_t maxlen);

ssize_t Readline_buffered (int fd, void *ptr, size_t maxlen);

ssize_t Readline_unbuffered (int fd, void *ptr, size_t maxlen);

ssize_t readline (int fd, void *vptr, size_t maxlen);

ssize_t Readline (int fd, void *ptr, size_t maxlen);

ssize_t writen(int fd, const void *vptr, size_t n);

void Writen (int fd, void *ptr, size_t nbytes);

ssize_t sendn (int fd, const void *vptr, size_t n, int flags);

void Sendn (int fd, void *ptr, size_t nbytes, int flags);

int Select (int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);

pid_t Fork (void);

struct hostent *Gethostbyname (const char *hostname);

void Getsockname (int sockfd, struct sockaddr *localaddr, socklen_t *addrp);

void Getaddrinfo ( const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

void
Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr);

void
Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr);

void
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

char *
sock_ntop(const struct sockaddr *sa, socklen_t salen);

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen);

char *
sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

char *
Sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

void
Fclose(FILE *fp);

char *
Fgets(char *ptr, int n, FILE *stream);

FILE *
Fopen(const char *filename, const char *mode);

void
Fputs(const char *ptr, FILE *stream);

Sigfunc * 
signal(int signo, Sigfunc *func);

Sigfunc * 
Signal(int signo, Sigfunc *func);

void
showAddr(char *str, struct sockaddr_in *a);

#endif
