/* websys.h
 *
 * Part of the Webio Open Source lightweight web server.
 *
 * Copyright (c) 2007 by John Bartas
 * All rights reserved.
 *
 * Use license: Modified from standard BSD license.
 * 
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation, advertising 
 * materials, Web server pages, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by John Bartas. The name "John Bartas" may not be used to 
 * endorse or promote products derived from this software without 
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef _WEBSYS_H_
#define _WEBSY_H_    1

/* This file contains definitions intended for modification during porting */

/*********** Optional webio features - comment out to remove ***************/

#define WI_STDFILES  1     /* Use system "fopen" files */
#define WI_EMBFILES  1     /* Use embedded FS */

#define WI_THREAD    1     /* Drive webio with a thread rather than polling */


/*********** Webio sizes and limits ***************/

#define WI_RXBUFSIZE    1536  /* rxbuf[] total size */
#define WI_TXBUFSIZE    1400  /* txbuf[] section size */
#define WI_MAXURLSIZE   512   /* URL buffer size  */
#define WI_FSBUFSIZE    4096  /* file read buffer size */

#define WI_PERSISTTMO   300   /* persistent connection timeout */

/*********** Network portability ***************/

#ifdef LINUX

#define socktype  long

typedef unsigned long u_long;

//#include <sys/timex.h>   /* For struct timeval */
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define WI_NOBLOCKSOCK(socket) fcntl(socket, F_SETFL, O_NONBLOCK)

#define closesocket(socket) close(socket)

/* Hard to believe the Posix kids renamed stricmp() */
#ifndef stricmp
#define stricmp(s1,s2) strcasecmp(s1,s2)
#endif

//extern void * malloc(int size);

#ifdef LINUX_DEMO

extern u_long cticks;
#define TPS	10		// ticks per second


#define TH_SLEEP(ticks) usleep(ticks * 100000)

#endif

#else /*not LINUX  */

/* Windows (winsock) version */

/* Some UNIX & Linux systems have an in_addr copnvention of using
 *  S_un.S_addr for the uint32 vfersion of an IP address.
 * On Centos this seems to have been "simplified" away in a way
 * That can't be ifdeffed (Stupid) so this expression has been 
 * changed in the source to the New-fnagled s_addr. The
 * define below might get it back.
 */

#define s_addr S_un.S_addr 

#ifdef BUSTER
#include "busterport.h"
#pragma warning(disable: 4152 )
extern   int      WI_NOBLOCKSOCK(long sock);
#else  /* not BUSTER, windows demo */

#include <winsock.h>
extern int WI_NOBLOCKSOCK(socktype sock);

#endif  /* BUSTER or not */

#ifdef errno
#undef errno
#endif
#define errno WSAGetLastError()

#define socktype  long


#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED

#endif /* LINUX or not */

/*********** Macros to system code ***************/

/* Map Webio heap routine to system's */
#define WI_MALLOC(size)     malloc(size)
#define WI_FREE(size)       free(size)

#ifndef BUSTER
#include <memory.h>
#define MEMCPY(dest,src,size)    memcpy(dest,src,size)
#endif

/*********** File system mapping ***************/

#define  USE_EMFILES 1
#define  USE_SYSFILES 1
#include <stdio.h>

/*********** debug support **************/

#ifndef BUSTER
extern   void     ws_dtrap();
#define  dtrap()  ws_dtrap()
#define  dprintf  printf

#include <stdarg.h>

#ifndef USE_ARG
#define USE_ARG(c) (c=c)
#endif  /* USE_ARG */
#endif  /* BUSTER */

void panic(char * msg);


#endif   /* _WEBSYS_H_ */


