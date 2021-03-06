/* webobjs.c
 *
 * Part of the Webio Open Source lightweight web server.
 *
 * Copyright (c) 2007-2009 by John Bartas
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

#include "websys.h"
#include "webio.h"
#include "webfs.h"

#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <unistd.h>
#endif

/* This file contins the code to manage webio objects, such as sessions,
 * tx buffers, and forms. It also contains heap "wrappers" with error
 * checking.
 */

wi_sess * wi_sessions;     /* Master list of sessions */

u_long   wi_blocks = 0;
u_long   wi_bytes = 0;
u_long   wi_maxbytes = 0;
u_long   wi_totalblocks = 0;


/* Webio's heap system allocates a bit more memory from the system
 * heap than the size passed. The extra space contains the ascii for
 * pattern "MEMM". The front marker also has a length field so we
 * can find the back marker.
 */

#ifdef WI_USE_MALLOC

int   wi_marker = 0x4D454D4D;    /* MEMM */

struct memmarker {
   int   marker;
   int   msize;   /* size of this block */
};

char * wi_alloc(int bufsize) {
   char * buffer;
   struct memmarker * mark;
   int   totalsize;

   totalsize = bufsize + sizeof(struct memmarker) + 4;

   buffer = WI_MALLOC(totalsize);
    /* Bugfix from PB 03/07/2009 17.58.07 */
   if (buffer == NULL) {
	   return (NULL);
   }

   memset(buffer, 0, totalsize);

   mark = (struct memmarker * )buffer;
   mark->marker = wi_marker;        /* Mark beginning of buffer */
   mark->msize = bufsize;
   buffer = (char*)(mark + 1);      /* get return value */
   *(int*)(buffer + bufsize) = wi_marker;    /* Mark end of buffer */

   wi_blocks++;
   wi_totalblocks++;
   wi_bytes += bufsize;
   if (wi_bytes > wi_maxbytes)
      wi_maxbytes = wi_bytes;

   return buffer;
}

void wi_free(void * buff) {
   struct memmarker * mark;
   char * cp;

   /* first, find the lead marker and check for overwritting */
   mark = (struct memmarker *)buff;
   mark--;        /* marker is prepended to buffer */

   /* check for corruption of pre-buffer area */
   if (mark->marker != wi_marker)
      wi_panic("wi_free: pre");
   
   /* check for corruption of post-buffer area */
   cp = (char*)buff;
   if ( *(int*)(cp + mark->msize) != wi_marker)
      wi_panic("wi_free: post");
   
   wi_blocks--;
   wi_bytes -= mark->msize;

   WI_FREE( (void*)mark );
}

#endif

/* txbuf constructor */

#ifndef WI_USE_MALLOC
static txbuf wi_txbuf_slot[MAX_TXBUF_SLOTS];
static u_char wi_txbuf_slot_used[MAX_TXBUF_SLOTS];

txbuf * wi_get_txbuf_slot(void) {
	int i;
	txbuf * newtxbuf = NULL;
	for (i = 0; i < MAX_TXBUF_SLOTS; ++i) {
		if (wi_txbuf_slot_used[i] == 0) {
			wi_txbuf_slot_used[i] = 1;
			newtxbuf = &wi_txbuf_slot[i];
			memset(newtxbuf,0,sizeof(txbuf));
			//dprintf("Acq TxBuf[%u]\n", (unsigned int)i);
			break;
		}
	}
	return newtxbuf;
}

void wi_free_txbuf_slot(txbuf * oldtxbuf) {
	int i;
	for (i = 0; i < MAX_TXBUF_SLOTS; ++i) {
		if ((oldtxbuf == &wi_txbuf_slot[i]) && (wi_txbuf_slot_used[i] != 0)) {
			wi_txbuf_slot_used[i] = 0;
			//dprintf("Free TxBuf[%u]\n", (unsigned int)i);
			break;
		}
	}
}
#endif

txbuf * wi_txalloc(wi_sess * websess) {
   txbuf * newtx;

#ifdef WI_USE_MALLOC
   newtx = (txbuf*)wi_alloc( sizeof(txbuf) );
#else
   newtx = wi_get_txbuf_slot();
#endif

   if (!newtx) {
	   return NULL;
   }

   /* Install new TX buffer at end of session chain */
   if (websess->ws_txtail) {
	   websess->ws_txtail->tb_next = newtx;   /* add to existing tail */
   }

   websess->ws_txtail = newtx;      /* new buffer is new tail */

   /* If empty, also make it head */
   if (websess->ws_txbufs == NULL) {
	   websess->ws_txbufs = newtx;
   }

   newtx->tb_session = websess;     /* backpointer to session */

   return newtx;
}

/* txbuf destructor */

void wi_txfree(txbuf * oldtx) {
   wi_sess *   websess;
   txbuf *     tmptx;
   txbuf *     last;

   /* This has most likely been unlinked already, but try to find it
    * in the sesion queue just in case.
    */
   websess = oldtx->tb_session;
   last = NULL;
   for (tmptx = websess->ws_txbufs; tmptx; tmptx = tmptx->tb_next) {
      if (tmptx == oldtx) {
         if (last) {
        	 last->tb_next = oldtx->tb_next;
         } else {
        	 websess->ws_txbufs = oldtx->tb_next;
         }
         break;
      }
   }

#ifdef WI_USE_MALLOC
   wi_free(oldtx);
#else
   wi_free_txbuf_slot(oldtx);
#endif
   return;
}


/* wi_sess constructor */

#ifndef WI_USE_MALLOC
static wi_sess wi_sess_slot[MAX_SESS_SLOTS];
static u_char wi_sess_slot_used[MAX_SESS_SLOTS];

wi_sess * wi_get_sess_slot(void) {
	int i;
	wi_sess * newsess = NULL;
	for (i = 0; i < MAX_SESS_SLOTS; ++i) {
		if (wi_sess_slot_used[i] == 0) {
			wi_sess_slot_used[i] = 1;
			newsess = &wi_sess_slot[i];
			memset(newsess,0,sizeof(wi_sess));
			//dprintf("Acq Sess[%u]\n", (unsigned int)i);
			break;
		}
	}
	return newsess;
}

void wi_free_sess_slot(wi_sess * oldsess) {
	int i;
	for (i = 0; i < MAX_SESS_SLOTS; ++i) {
		if ((oldsess == &wi_sess_slot[i]) && (wi_sess_slot_used[i] != 0)) {
			wi_sess_slot_used[i] = 0;
			//dprintf("Free Sess[%u]\n", (unsigned int)i);
			break;
		}
	}
}
#endif

#ifndef WI_USE_MALLOC
static wi_form wi_form_slot[MAX_FORM_SLOTS];
static u_char wi_form_slot_used[MAX_FORM_SLOTS];

wi_form * wi_get_form_slot(void) {
	int i;
	wi_form * newform = NULL;
	for (i = 0; i < MAX_FORM_SLOTS; ++i) {
		if (wi_form_slot_used[i] == 0) {
			wi_form_slot_used[i] = 1;
			newform = &wi_form_slot[i];
			memset(newform,0,sizeof(wi_form));
			//dprintf("Acq Form[%u]\n", (unsigned int)i);
			break;
		}
	}
	return newform;
}

void wi_free_form_slot(wi_form * oldform) {
	int i;
	for (i = 0; i < MAX_FORM_SLOTS; ++i) {
		if ((oldform == &wi_form_slot[i]) && (wi_form_slot_used[i] != 0)) {
			wi_form_slot_used[i] = 0;
			//dprintf("Free Form[%u]\n", (unsigned int)i);
			break;
		}
	}
}
#endif

wi_sess * wi_newsess(void) {
   wi_sess * newsess;

#ifdef WI_USE_MALLOC
   newsess = (wi_sess *)wi_alloc( sizeof(wi_sess) );
#else
   newsess = wi_get_sess_slot();
#endif

   if (!newsess) {
      dprintf("wi_newsess: out of memory.\n");
      return NULL;
   }
   newsess->ws_socket = INVALID_SOCKET;
   newsess->ws_state = WI_HEADER;
   newsess->ws_last = wi_cticks;

   /* Add new session to master list */
   newsess->ws_next = wi_sessions;
   wi_sessions = newsess;

   /* All new sessions strt out ready to read their socket */
   newsess->ws_flags |= WF_READINGCMDS;

   return newsess;
}


/* wi_sess destructor */

void wi_delsess(wi_sess * oldsess) {
   wi_sess * tmpsess;
   wi_sess * lastsess;

   if (oldsess->ws_socket != INVALID_SOCKET) {
      closesocket(oldsess->ws_socket);
      oldsess->ws_socket = 0;
   }

   /* Unlink from master session list */
   lastsess = NULL;
   for (tmpsess = wi_sessions; tmpsess; tmpsess = tmpsess->ws_next) {
      if (tmpsess == oldsess) { /* Found session to unlink? */
         if (lastsess) {
        	 lastsess->ws_next = tmpsess->ws_next;
         } else {
        	 wi_sessions  = tmpsess->ws_next;
         }
         break;
      }
      lastsess= tmpsess;
   }

   /* Make sure there are no dangling resources */
   if (oldsess->ws_txbufs) {
      while (oldsess->ws_txbufs) {
         wi_txfree(oldsess->ws_txbufs);
      }
   }
   if (oldsess->ws_filelist) {
      if (oldsess->ws_filelist) {
         wi_fclose( oldsess->ws_filelist);
      }
   }
   /* Fix submitted by PB     09/11/2009 18.39.29 */
   if (oldsess->ws_formlist) {
       struct wi_form_s * nextform;
       while (oldsess->ws_formlist) {
           nextform = oldsess->ws_formlist->next;
#ifdef WI_USE_MALLOC
           wi_free(oldsess->ws_formlist);
#else
           wi_free_form_slot(oldsess->ws_formlist);
#endif
           if (nextform) {
               dtrap(); // check double-form first time through...
           }
           oldsess->ws_formlist = nextform;
       }
   }  
   /*     PB     09/11/2009 18.39.29 */

#ifdef WI_USE_MALLOC
   wi_free(oldsess);
#else
   wi_free_sess_slot(oldsess);
#endif

   return;
}

/* wi_file constructor */

#ifndef WI_USE_MALLOC
static wi_file wi_file_slot[MAX_FILE_SLOTS];
static u_char wi_file_slot_used[MAX_FILE_SLOTS];

wi_file * wi_get_file_slot(void) {
	int i;
	wi_file * newfile = NULL;
	for (i = 0; i < MAX_FILE_SLOTS; ++i) {
		if (wi_file_slot_used[i] == 0) {
			wi_file_slot_used[i] = 1;
			newfile = &wi_file_slot[i];
			memset(newfile,0,sizeof(wi_file));
			//dprintf("Acq File[%u]\n", (unsigned int)i);
			break;
		}
	}
	return newfile;
}

void wi_free_file_slot(wi_file * oldfile) {
	int i;
	for (i = 0; i < MAX_FILE_SLOTS; ++i) {
		if ((oldfile == &wi_file_slot[i]) && (wi_file_slot_used[i] != 0)) {
			wi_file_slot_used[i] = 0;
			//dprintf("Free File[%u]\n", (unsigned int)i);
			break;
		}
	}
}
#endif

wi_file * wi_newfile(wi_filesys * fsys, wi_sess * sess, void * fd) {
   wi_file *      newfile;

#ifdef WI_USE_MALLOC
   newfile = (wi_file *)wi_alloc( sizeof(wi_file));
#else
   newfile = wi_get_file_slot();
#endif

   if (!newfile) {
	   return NULL;
   }

   newfile->wf_fd = fd;
   newfile->wf_routines = fsys;
   newfile->wf_sess = sess;

   /* Put new file at front of session file list */
   newfile->wf_next = sess->ws_filelist;
   sess->ws_filelist = newfile;

   return newfile;
}


/* wi_file destructor */

int wi_delfile(wi_file * delfile) {
   wi_sess *   sess;
   wi_file *   tmpfi;
   wi_file *   last;

   /* unlink file from session list */
   sess = delfile->wf_sess;
   last = NULL;
   for (tmpfi = sess->ws_filelist; tmpfi; tmpfi = tmpfi->wf_next) {
      if (tmpfi == delfile) {
         if (last) {
        	 last->wf_next = delfile->wf_next;
         } else {
        	 sess->ws_filelist = delfile->wf_next;
         }
         break;
      }
      last = tmpfi;
   }

#ifdef WI_USE_MALLOC
   wi_free(delfile);
#else
   wi_free_file_slot(delfile);
#endif

   return 0;
}

