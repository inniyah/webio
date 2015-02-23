rem build embedded FS for Buster
rem Get the latest Windows FS compiler
copy ..\fsbuild\debug\fsbuild.exe .

fsbuild -g wsfcode.c filelist

