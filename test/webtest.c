/* webio.c
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

/* This file is only for testing or making a plain standalone web
 * server app. It is not needed in a web server when the server 
 * is built as a library for inclusion in an application
 * program or an embedded system.
 */

#include "websys.h"
#include "webio.h"
#include "webfs.h"
#include "wsfdata.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void exit(int code);

/* Sample authentication code & "database" */
static const char test_name[32] = {"test"};
static const char test_passwd[32] = {"test"};

int wfs_auth(void * fd, const char * name, const char * password);

u_long wi_cticks = 0;

char * usage = "only valid command line arg is a TCP port number for listening\n";

int main( int argc, char * argv[] ) {
   int error;

   dprintf("Webio server starting...\n");

#ifdef _WINSOCKAPI_
   {
      WSADATA wsaData;
      error = WSAStartup(MAKEWORD(2,2), &wsaData);
      if (error)
      {
         dprintf("windsuck startup error %d\n", error);
         exit(EXIT_FAILURE);
      }
   }
#endif

   if (argc > 1) {
	    httpport = atoi(argv[1]);
		if (httpport < 1) {
			dprintf("%s", usage);
			dtrap();
			exit(EXIT_FAILURE);
		}
   }


   error = wi_init();
   if (error < 0) {
      dprintf("wi_init error %d\n", error);
	  if (error == WI_E_SOCKET) {
	      dprintf("socket error %d\n", errno);
	  }
	  dtrap();
      exit(EXIT_FAILURE);
   }

   /* Install our port-local authentication routine */
   emfs.wfs_fauth = wfs_auth;


   error = wi_thread();   /* blocks here until killed */
   if (error < 0) {
      dprintf("wi_init error %d\n", error);
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}


/* Return true if user gets access to the embedded file, 0 if not. */

int wfs_auth(void * fd, const char * name, const char * password) {

   /* See if this file requires authentication. */
   EOFILE *    eofile;
   em_file *   emf;

   eofile = (EOFILE *)fd;
   emf = eofile->eo_emfile;

   if (emf->em_flags & EMF_AUTH) {
      if (stricmp(name, test_name)) {
    	  return 0;
      }
      if (stricmp(password, test_passwd)) {
    	  return 0;
      }
   }
   return 1;
}


void wi_dtrap(void) {
   dprintf("dtrap - need breakpoint");
}

void wi_panic(char * msg) {
   dprintf("wi_panic: %s", msg);
   dtrap();
   exit(EXIT_FAILURE);
}

/* memory_ssi()
 *
 * Sample SII routine to dump memory statistics into an output html file. 
 *
 */

extern u_long   wi_blocks;
extern u_long   wi_bytes;
extern u_long   wi_maxbytes;
extern u_long   wi_totalblocks;


int memory_ssi(wi_sess * sess, EOFILE * eofile) {
   /* print memory stats to the session's TX buffers */
   wi_printf(sess, "Current blocks: %d <br>", wi_blocks );
   wi_printf(sess, "Current bytes: %d <br>", wi_bytes );
   wi_printf(sess, "Total blocks: %d <br>", wi_totalblocks );
   wi_printf(sess, "Max. bytes: %d <br>", wi_maxbytes );
   return 0;      /* OK return code */
}

int wi_cvariables(wi_sess * sess, int token){
   int e = 0;

   switch(token) {
   case MEMHITS_VAR5:
		  e = wi_putlong(sess, (u_long)(wi_totalblocks));
		  break;
   }
   return e;
}


/* testaction_cgi
 * Stub routine for form processing
 * 
 * Returns 0 if OK,else negative error code
 */

char * testaction_cgi(wi_sess * sess,  EOFILE * eofile) {
   char *   your_name;
   your_name = wi_formvalue(sess, "your_name");   /* default: John */
   (void)your_name;

   dprintf("testaction.cgi: your_name=%s\n", your_name);

   if ( wi_redirect(sess, "index.html") ) {
	   	return("redir failed");
   } else {
	   	return NULL;
   }
}

/* PUSH
 *
 * pushtest_func routine stub
 */

int pushtest_func(wi_sess * sess, EOFILE * eofile) {
   /* Add your code here */
   return 0;
}

