libndofdev
==========

Linux support for the 3Dconnexion SpaceBall, SpaceNavigator and joysticks in the SecondLife clients

Currently supported are the following devices:

VID:PID
-------
0x046d:0xc626   3Dconnexion SpaceNavigator
0x046d:0xc621   3Dconnexion SpaceBall 5000
0x046d:0xc623   3Dconnexion SpaceTraveller (untested)
0x046d:0xc603   3Dconnexion SpaceMouse (untested)

In addition, the library supports every joystick/gamepad device supported by the SDL library.


C A U T I O N !!!
-----------------

DO NOT USE THIS LIBRARY FOR ADDING SUPPORT FOR THE 3Dconnexion HARDWARE TO NEW APPLICATIONS!

This library is a messy hack emulating the proprietary 3Dconnexion SDK, in order to simplify the SecondLife 
client integration. If you are looking to integrate support for these devices into a new application, 
a much better solution is to use the event interface directly. An example of how to do this can be found
here: 
http://janoc.rd-h.com/files/software/linux/spacenav/spacenavig.c

If you are building a virtual reality application, then the best option is to use the VRPN library: 
http://www.cs.unc.edu/Research/vrpn/
VRPN supports all the devices above and many more in a transparent and abstract manner, so you don't have 
to add support for each different interaction device or tracker specially. 

