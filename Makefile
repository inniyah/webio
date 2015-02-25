all: fsbuilder webio

LIB_NAME=webio
LIB_MAJOR=0
LIB_MINOR=1

LIBRARY=lib$(LIB_NAME)

LIB_OBJS = \
	obj/webclib.o \
	obj/webfs.o \
	obj/webio.o \
	obj/webobjs.o \
	obj/websys.o \
	obj/webutils.o

TEST_OBJS = \
	obj/htmldata.o \
	obj/imgdata.o \
	obj/webtest.o

CFLAGS= -O2 -g -Wall
LDFLAGS=
#LDFLAGS= -Wl,-z,defs -Wl,--as-needed -Wl,--no-undefined
DEFS=-DLINUX -DLINUX_DEMO
LIBS=
INCS=-Isrc -Idata

# Use system "fopen" files
DEFS+=-DWI_USE_STDFILES

# Use embedded FS
DEFS+=-DWI_USE_EMBFILES

# Drive webio with a thread rather than polling
DEFS+=-DWI_USE_THREADS

# Debug messages
DEFS+=-DWI_USE_DPRINTF

# Use dynamic memory from the Heap
DEFS+=-DWI_USE_MALLOC

# Configuration parameters when no heap memory is used
#DEFS += -DMAX_TXBUF_SLOTS=4 -DMAX_SESS_SLOTS=4 -DMAX_EOFILE_SLOTS=16 -DMAX_FILE_SLOTS=16 -DMAX_FORM_SLOTS=4 -DMAX_FORM_PARAMS=16  



SHARED_LIB_OBJS = $(LIB_OBJS:.o=.shared.o)
STATIC_LIB_OBJS = $(LIB_OBJS:.o=.static.o)

$(LIBRARY).so.$(LIB_MAJOR).$(LIB_MINOR): $(SHARED_LIB_OBJS)
	g++ $(LDFLAGS) $(EXTRA_LDFLAGS) -shared \
		-Wl,-soname,$(LIBRARY).so.$(LIB_MAJOR) \
		-o $(LIBRARY).so.$(LIB_MAJOR).$(LIB_MINOR) \
		$+ -o $@ $(LIBS)

$(LIBRARY).so: $(LIBRARY).so.$(LIB_MAJOR).$(LIB_MINOR)
	rm -f $@.$(LIB_MAJOR)
	ln -s $@.$(LIB_MAJOR).$(LIB_MINOR) $@.$(LIB_MAJOR)
	rm -f $@
	ln -s $@.$(LIB_MAJOR) $@

$(LIBRARY).a: $(STATIC_LIB_OBJS)
	ar cru $@ $+

obj/%.shared.o: src/%.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS) -fPIC

obj/%.shared.o: src/%.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS) -fPIC

obj/%.static.o: src/%.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.static.o: src/%.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)



webio: $(LIB_OBJS) $(TEST_OBJS)
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)



fsbuilder: fsbuild/fsbuilder.o
	g++ $(LDFLAGS) $+ -o $@ $(LIBS)



data/imgdata.o: data/imgdata.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

data/imgdata.c: fsbuilder data/snail.gif data/prlogo.gif data/filelist
	cd data && ../fsbuilder -o htmldata.c filelist

data/htmldata.o: data/htmldata.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

data/htmldata.c: fsbuilder data/index.html data/filelist
	cd data && ../fsbuilder -o htmldata.c filelist

obj/%.o: src/%.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.o: src/%.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.o: data/%.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.o: data/%.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.o: test/%.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

obj/%.o: test/%.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

%.o: %.cpp
	g++ -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)

%.o: %.c
	gcc -o $@ -c $(DEFS) $(INCS) $+ $(CFLAGS)


clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS)
	rm -f webio fsbuilder
	rm -f data/imgdata.c data/htmldata.c data/wsfcode.c data/wsfdata.h
	rm -f *.o */*.o *.a */*.a *.so *.so.* *~
