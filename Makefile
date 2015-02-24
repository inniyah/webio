all: fsbuilder webio

INCUDES = webfs.h  webio.h  websys.h

OBJS = \
	htmldata.o \
	webclib.o \
	webfs.o \
	webio.o \
	webobjs.o \
	websys.o \
	webtest.o \
	webutils.o \
	imgdata.o

CFLAGS= -O2 -g -Wall
LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
DEFS=-DLINUX -DLINUX_DEMO
LIBS=
INCS=

# Use system "fopen" files
DEFS+=-DWI_STDFILES

# Use embedded FS
DEFS+=-DWI_EMBFILES

# Drive webio with a thread rather than polling
DEFS+=-DWI_THREAD

webio: $(OBJS)
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)

fsbuilder: fsbuild/fsbuilder.o
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)

imgdata.o: imgdata.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

imgdata.c: fsbuilder snail.gif prlogo.gif filelist
	./fsbuilder -o htmldata.c filelist

htmldata.o: htmldata.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

htmldata.c: fsbuilder index.html filelist
	./fsbuilder -o htmldata.c filelist

%.o: %.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

%.o: %.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)


clean:
	rm -f $(OBJS)
	rm -f webio fsbuilder
	rm -f imgdata.c htmldata.c
	rm -f *.o */*.o *.a */*.a *~
