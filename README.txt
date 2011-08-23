How to build VstBoard :

Steinberg SDKs :
================
Steinberg VST and ASIO SDKs can't be redistributed : you have to create an account and download them yourself.
We need "VST 2.4 Audio Plug-Ins SDK" and "ASIO SDK" available at http://www.steinberg.net/en/company/developer.html
	+ Copy the VST SDK in ./libs/vstsdk2.4 (see README.txt in that folder)
	+ Copy the Asio SDK in ./libs/portaudio/src/hostapi/asio/ASIOSDK (see README.txt in that folder)

Qt :
============
You'll need :
	+ Qt libraries for Windows (VS 2008)   (didn't checked GCC lately, but it's supposed to work)
	+ Qt Creator
Or get the full SDK from http://qt.nokia.com/downloads

Build la chose :
============
QtWinMigrate must be built first, the compiled files will be added in your Qt directory, in bin/ :
	+ compile libs/qtwinmigrate/buildlib/buildlib.pro
Check the directories in the config file :
	+ mainconfig.pri
	+ compile scr/builall.pro

