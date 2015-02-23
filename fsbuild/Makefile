# Linux Makefile for webio fsbuilder utility
# 
# 
# 
# 


DEBUG = -g

fsbuilder: fsbuilder.cpp Makefile
	g++ $(DEBUG) fsbuilder.cpp -o fsbuilder

clean:
	rm -f fsbuilder.o fsbuilder

test:
	 ./fsbuilder  filelist