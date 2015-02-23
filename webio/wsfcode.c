/* wsfcode.c
 * 
 * This file was created by an automated utility. 
 * It is not intended for manual editing
 */

#include "websys.h"
#include "webio.h"
#include "webfs.h"
#include "wsfdata.h"



/* memory_ssi()
 *
 * SSI routine stub
 */

int
memory_ssi(wi_sess * sess, EOFILE * eofile)
{
   /* Add your code here */
   return 0;
}



/* pushtest_func()
 *
 * PUSH routine stub
 */

int
pushtest_func(wi_sess * sess, EOFILE * eofile)
{
   /* Add your code here */
   return 0;
}


/* testaction_cgi
 * Stub routine for form processing
 * 
 * Returns NULL if OK, else short error text
 */

char *
testaction_cgi(wi_sess * sess, EOFILE * eofile)
{
   char *   your_name;

   your_name = wi_formvalue(sess, "your_name");   /* default: John */


   /* Add your code here */

   return 0;
}


int
wi_cvariables(wi_sess * sess, int token)
{
   int   e;

   switch(token)
   {
   case MEMHITS_VAR5:
      e = wi_putlong(sess, (u_long)(wi_totalblocks));
      break;
   default:
      e = WIE_BADPARM;
      break;
   }
   return e;
}
