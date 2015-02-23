#-------------------------------------------------------------------------------
# Makefile for ./webio  Buster directory
#

include ../settings.mk

WEBIO_OBJS = wsfcode.o webclib.o webfs.o webio.o webobjs.o	\
	websys.o webtest.o webutils.o


.c.o:
	$(CC) $(CC_FLAGS) $*.c


$(WEBIOLIB): $(WEBIO_OBJS)



clean:
	rm *.o 
	rm *.*~ 


