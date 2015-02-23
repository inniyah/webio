: clean.bat for Visual Studio 2010 project directories
:
: Visual Studio 2010 built-in clean function only gets 
: about half the bloat. 
:

del *.sdf
del debug/*
del *.ncb
rmdir /s /q ipch
