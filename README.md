# Webio

Webio is a small-footprint web server, designed for embedding in an application or an embedded system. It includes a tool for generating embedded SSI and CGI routines, and a detailed programmers manual.

Readme file for Webio embedded web server directory.

	Last Edit Dec. 31st, 2011

Webio sources are currently available at codeproject.com, daddywall.com, 
and soon sourceforge.com. Once we find someone will to donate a Subversion 
server we'll start publishing specific version numbers. Until then just find
source and documentation that seems to have consistent dates across the system.

	The latest and greatest is currently at:

http://www.daddywall.com/webio/webio-dec2011.zip

Detailed documentation is in the file progman.html, which should have been included 
with sources and samples.  This file fills in update info which may not make it
into the html file. 

During 2011 the demo application was built and run on Centos Linux 5.6 and 
Windows & using Microsoft Visual Studio 2010. Project files for Visual 
Studio 6.0 are preserved with "-60" added to the main filename, i.e. 
webio.dsp for MSVC 6.0 is named webio-60.dsp. These files have not been tested
in a few years - use at your own risk.

A new Linux makefile has been added so that both the demo and a library can be 
built int he same directory. The demo is built be the default "makefile",  the
library (at least the "daddywall" version) can be built be dwallweb.mk. Since the 
Daddywall sources are not open the latter file is purely informational. 

	Known bugs & problems:

* The cticks counter is not maintained in the demo programs.

* The -c flag (for cache control) is documented but not supported. 

* On at least some Windows 7 machines, bind fails on port 80. It seems Microsoft 
used it for their proprietary "IIS".  To get around this the default port for 
windows builds is changed to 88, thus your browser can access your web 
server's pages at http://127.0.0.1:80 More proof that if Bill Gates 
pair a nickel to every programmer who had to work around a 
Microsoft bug he would be broke.

