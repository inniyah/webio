/* webfs.c
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

#include "websys.h"
#include "webio.h"
#include "webfs.h"

#include <string.h>

/* This file contins webio file access routines. The external "wi_f" entry 
 * points have the same semantics as C buffered file IO (fopen, etc). These 
 * determine which of the actual file systems should be called and make 
 * the call.
 */

#ifdef WI_USE_STDFILES
/* Voiding these pointers is ugly, but so are the Linux declarations. */
wi_filesys sysfs = {
	.wfs_fopen  = em_fopen,
	.wfs_fread  = em_fread,
	.wfs_fwrite = em_fwrite,
	.wfs_fclose = em_fclose,
	.wfs_fseek  = em_fseek,
	.wfs_ftell  = em_ftell
};
#endif   /* WI_USE_STDFILES */

#ifdef WI_USE_EMBFILES
wi_filesys emfs = {
   em_fopen,
   em_fread,
   em_fwrite,
   em_fclose,
   em_fseek,
   em_ftell
};
#endif   /* WI_USE_EMBFILES */

/* Table ofthe supported file systems */
wi_filesys * wi_filesystems[] = { /* list of file systems */
#ifdef WI_USE_EMBFILES
   &emfs,
#endif
#ifdef WI_USE_STDFILES
   &sysfs,
#endif
   NULL     /* reserved for runtime entry */
};


wi_file *      wi_allfiles;   /* list of all open files */


/* wi_fopen()
 * 
 * webio top level file open routine. This is just wrapper for the lower
 * level routine - either the Embedded FS, and the host system's native FS.
 *
 * Returns: 0 if OK else negative WI_E_ error code.
 * 
 */

int wi_fopen(wi_sess * sess, const char * name, const char * mode) {
   wi_filesys *   fsys;    /* File system which has the file */
   wi_file *      newfile; /* transient file structrure */
   void *         fd;      /* descriptior from fs */
   int            i;

   /* Loop through the FS list, trying an open on each */
   for (i = 0; i < sizeof(wi_filesystems)/sizeof(wi_filesys*); i++) {
      fsys = wi_filesystems[i];
      if (fsys == NULL) {
    	  continue;
      }
      fd = fsys->wfs_fopen(name, mode);
      if (fd) {
         /* Got an open - create a wi_file & fill it in. */
         newfile = wi_newfile(fsys, sess, fd);
         if (!newfile) {
            fsys->wfs_fclose(fd);
            return WI_E_MEMORY;
         }
         return 0;
      }
   }
   return WI_E_NOFILE;
}


int wi_fread(char * buf, unsigned size1, unsigned size2, void * filep) {
   int   bytes;
   WI_FILE * fd;
   fd = (WI_FILE *)filep;
   bytes = fd->wf_routines->wfs_fread(buf, size1, size2, fd->wf_fd);
   return bytes;
}

int wi_fwrite(char * buf, unsigned size1, unsigned size2, void * filep) {
   int   bytes;
   WI_FILE * fd;
   fd = (WI_FILE *)filep;
   bytes = fd->wf_routines->wfs_fwrite(buf, size1, size2, fd->wf_fd);
   return bytes;
}


int wi_fclose(WI_FILE * fd) {
   int   error;

   /* close file at lower level, get an error code */
   error = fd->wf_routines->wfs_fclose(fd->wf_fd);

   /* Delete our intermediate layer struct for this file. */
   wi_delfile(fd);

   return error;    /* return error from lower layer delete */
}


int wi_fseek(WI_FILE * fd, long offset, int mode) {
   return(fd->wf_routines->wfs_fseek(fd->wf_fd, offset, mode));
}


int wi_ftell(WI_FILE * fd) {
   return(fd->wf_routines->wfs_ftell(fd->wf_fd));
}

/***************** Optional embedded FS starts here *****************/
#ifdef WI_USE_EMBFILES

extern em_file efslist[];

/* Set up master list of embedded files. If "efslist[]" is an unresolved 
 * external when you link then you have neglected to provide the data 
 * for the embedded files. The normal way of doing this is to use the
 * HTML compiler to produce one or more C files containing the data.
 * this process will define and generate efslist[] for you.
 */
em_file * emfiles = &efslist[0];

/* transient list of em_ files which are currently open */
EOFILE * em_openlist;

/* em_verify()
 * 
 * Make sure a passed fd is really an EOFILE.
 * 
 * Returns 0 if it is, or WI_E_BADFILE if not.
 */
int em_verify(EOFILE * fd) {
   EOFILE *    eofile;

   /* verify file pointer is valid */
   for (eofile = em_openlist; eofile;eofile = eofile->eo_next) {
      if (eofile == fd) {
    	  break;
      }
   }
   if (!eofile) {
	   return WI_E_BADFILE;
   }

   return 0;
}

/* em_lookupsess()
 * 
 * Lookup web session based on an emf fd 
 *
 * returns session, or NULL if not found.
 */

wi_sess * em_lookupsess(void * fd) {
   wi_sess *   sess;

   for (sess = wi_sessions; sess; sess = sess->ws_next) {
      if (sess->ws_filelist->wf_fd == fd) {
    	  return sess;
      }
   }

   return NULL;
}

#ifndef WI_USE_MALLOC
static EOFILE wi_eofile_slot[MAX_EOFILE_SLOTS];
static u_char wi_eofile_slot_used[MAX_EOFILE_SLOTS];

EOFILE * wi_get_eofile_slot(void) {
	int i;
	EOFILE * newfile = NULL;
	for (i = 0; i < MAX_EOFILE_SLOTS; ++i) {
		if (wi_eofile_slot_used[i] == 0) {
			wi_eofile_slot_used[i] = 1;
			newfile = &wi_eofile_slot[i];
			memset(newfile,0,sizeof(EOFILE));
			//dprintf("Acq EOFILE[%u]\n", (unsigned int)i);
			break;
		}
	}
	return newfile;
}

void wi_free_eofile_slot(EOFILE * oldfile) {
	int i;
	for (i = 0; i < MAX_EOFILE_SLOTS; ++i) {
		if ((oldfile == &wi_eofile_slot[i]) && (wi_eofile_slot_used[i] != 0)) {
			wi_eofile_slot_used[i] = 0;
			//dprintf("Free EOFILE[%u]\n", (unsigned int)i);
			break;
		}
	}
}
#endif

WI_FILE * em_fopen(const char * name, const char * mode) {
   em_file *    emf;
   EOFILE *     eofile;
   const char * cmpname = name;

   /* Search the efs a name matching the one passed. */
   for (emf = emfiles; emf; emf = emf->em_next) {
      if (emf->em_name[0] != *cmpname) { // fast test of first char
         continue;
      }

      if (strcmp(emf->em_name, cmpname) == 0) {
    	  break;
      }
   }
   if (!emf) { /* If file not in list, return NULL */
      return NULL;
   }

   if ( *mode != 'r' ) { /* All files are RO,otherwise return NULL */
      return NULL;
   }

   /* We're going to open file. Allocate the transient control structure */

#ifdef WI_USE_MALLOC
   eofile = (EOFILE *)wi_alloc(sizeof(EOFILE));
#else
   eofile = wi_get_eofile_slot();
#endif

   if (!eofile) {
	   return NULL;
   }
   eofile->eo_emfile = emf;
   eofile->eo_position = 0;

   /* Add new open struct to open files list */
   eofile->eo_next = em_openlist;
   em_openlist = eofile;

   return ((WI_FILE*)eofile);
}

int em_fread(char * buf, unsigned size1, unsigned size2, void * fd) {
   unsigned    datalen;    /* length of data to move */
   EOFILE *    eofile;
   em_file *   emf;
   int         error;

   eofile = (EOFILE *)fd;
   error = em_verify(eofile);
   if (error) {
	   return error;
   }

   emf = eofile->eo_emfile;

   /* SSI and forms should not make it this far down the call chain */
   if (emf->em_flags & (EMF_SSI|EMF_FORM)) {
      dtrap();
      return 0;
   }

   datalen = size1 * size2;
   if (datalen > (emf->em_size - eofile->eo_position)) {
	   datalen =  emf->em_size - eofile->eo_position;
   }

   /* Check for position at End of File - EOF */
   if (datalen == 0) {
	   return 0;
   }

   memcpy(buf, &emf->em_data[eofile->eo_position], datalen);
   eofile->eo_position += datalen;

   return datalen;
}


int em_fwrite(char * buf, unsigned size1, unsigned size2, void * fd) {
   int error;

   dtrap();

   (void)buf;
   (void)size1;
   (void)size2;
   error = em_verify((EOFILE*)fd);
   if (error) {
	   return error;
   }
   return 0;
}


int em_fclose(void * voidfd) {
   EOFILE *    passedfd;
   EOFILE *    tmpfd;
   EOFILE *    last;

   passedfd = (EOFILE *)voidfd;

   /* verify file pointer is valid */
   last = NULL;
   for (tmpfd = em_openlist; tmpfd; tmpfd = tmpfd->eo_next) {
      if (tmpfd == passedfd) { /* If we found it, unlink */
         if (last) {
        	 last->eo_next = passedfd->eo_next;
         } else {
        	 em_openlist = passedfd->eo_next;
         }
         break;
      }
      last = tmpfd;
   }

   if (tmpfd == NULL) { /* fd not in list? */
      return WI_E_BADFILE;
   }

#ifdef WI_USE_MALLOC
   wi_free(passedfd);
#else
   wi_free_eofile_slot(passedfd);
#endif

   return 0;
}


int em_fseek(void * fd, long offset, int mode) {
   EOFILE *    emf;
   int         error;
   int         newpos;
   int         size;       /* Total size of em file */

   emf = (EOFILE *)fd;
   error = em_verify(emf);
   if (error)  {
	   return error;
   }

   /* Get file size into local variable */
   size = emf->eo_emfile->em_size;

   /* Figure out where new position should be */
   switch (mode) {
   case SEEK_SET:
      newpos = offset;
      break;
   case SEEK_END:
      newpos = size + offset;
      break;
   case SEEK_CUR:
      newpos = emf->eo_position + offset;
      break;
   default:
      wi_panic("em_fseek");
      newpos = 0;
      break;
   }

   /* Sanity check new position */
   if ((newpos < 0) || (newpos > size)) {
	   return WI_E_BADPARM;
   }

   emf->eo_position = newpos;
   return 0;
}

int em_ftell(void * fd) {
   EOFILE *    emf;
   int         error;

   emf = (EOFILE *)fd;
   error = em_verify(emf);
   if (error) {
	   return -1;
   }
   
   return(emf->eo_position);
}

int em_push(void * fd, wi_sess * sess) {
   int         error;
   EOFILE *    emf;
   PUSH_ROUTINE * pushfunc;

   emf = (EOFILE *)fd;
   if (emf->eo_emfile->em_routine == NULL) {
	   return WI_E_BADFILE;
   }

   /* call embedded files embedded function */
   dtrap();
   pushfunc = emf->eo_emfile->em_routine;
   error = pushfunc(sess, emf);

   return error;
}

#endif  /* WI_USE_EMBFILES */
