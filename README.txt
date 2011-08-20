How to build VstBoard :

Steinberg SDKs :
================
Steinberg VST and ASIO SDKs can't be redistributed : you have to create an account and download them yourself.
We need "VST 2.4 Audio Plug-Ins SDK" and "ASIO SDK" available at http://www.steinberg.net/en/company/developer.html
	+ Copy the VST SDK in ./libs/vstsdk2.4 (see README.txt in that folder)
	+ Copy the Asio SDK in ./libs/portaudio/src/hostapi/asio/ASIOSDK (see README.txt in that folder)

Qt :
============
You can download the full SDK, or just :
	+ Qt libraries for Windows (VS 2008)   (didn't checked GCC lately, but it's supposed to work)
	+ Qt Creator
from http://qt.nokia.com/downloads

Build la chose :
============
You need to build QtWinMigrate first, it's an addon for Qt, the compiled files will be added in you Qt directory, in bin/ :
	+ libs/qtwinmigrate/buildlib/buildlib.pro

Check the directories in :
	+ mainconfig.pri
	
Now you can build the main project :
	+ scr/builall.pro

