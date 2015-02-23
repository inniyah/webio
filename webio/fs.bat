rem Get the latest FS compiler
copy ..\fsbuild\debug\fsbuild.exe .

fsbuild -g wsfcode.c filelist

