# EggBot on ESP8266

## Info

An [EggBot](https://egg-bot.com/) is a machine for drawing on eggs. I built an [EggBot](https://www.thingiverse.com/thing:3431363) which uses 3D printed parts.
As I'm using an ESP8266 for controlling the steppers and the servos I couldn't use the orginal firmware. This repository contains the software for my EggBot, features are:

The project is based on:
- [EggDuino](https://github.com/schlion/EggDuino) - code ported to ESP8266,
- [EggEsp](https://github.com/M4GNV5/EggEsp) - www page (svg to GCODE conversion),
- [EggBot Inkscape plugin]http://wiki.evilmadscientist.com/Installing_software

Features:
- Implemented Eggbot-Protocol-Version 2.1.0
- Turn-on homing: switch-on position of pen will be taken as reference point.
- No collision-detection!!
- Supported Servos: standard analog mini servos like TG9e, SG90, ES08MA, HXT900, etc.
- Taking svgs directly (over www) -> no need for external software
- Inkscape plugin (commands over serial port or TCP socket on port 2500).

The test project was built on D1 mini board connected to PC computer over USB as /dev/ttyUSB0 under Linux.
Modify Wifi client settings in main.cpp file:
#define WIFI_SSID                "EggBot"
#define WIFI_PASS                "EggBotPass"

# Parts needed:
* D1 mini (ESP8266),
* DC/DC converter 12V to 5V,
* DC/DC converter 5V to 3.3V,
* 2 x TMC2208/A4988/equivalent,
* 2 x Stepper mottor Nema 17
* Servo SG90
* 3D printed and mechanical parts from [EggBot](https://www.thingiverse.com/thing:3431363)
* wires
* connectors

# D1 mini CONNECTIONS
* STEP1(EGG)- GPIO14 (D5)
* DIR1(EGG) - GPIO13 (D7)
* STEP2(ARM)- GPIO5  (D1)
* DIR2(ARM) - GPIO4  (D2)
* MOTTOR EN - GPIO2  (D4)
* SERVO     - GPIO12 (D6)
* BUTTON    - GPIO0  (D3)

# Wiring

![alt tag](https://github.com/BubuHub/EspEggBot/blob/main/blob/assets/schematic.png)

## Building

The project uses platformio build environment. 
[PlatformIO](https://platformio.org/) - Professional collaborative platform for embedded development.

* install PlatformIO
* enter project directory
* connect Webmos D1 mini board to PC computer over USB cable.
* type in terminal:
  platformio run
  platformio upload

You can also use IDE to build this project on Linux/Windows/Mac. My fvorite ones:
* [Code](https://code.visualstudio.com/) 
* [Atom](https://atom.io/)

## Inkscape plugin installation

Installation:
- Install Inkscape Tools wit Eggbot extension. Detailed instructions: (You just need to complete Steps 1 and 2)
http://wiki.evilmadscientist.com/Installing_software

- D1 mini cannot be detected by default by the Eggbot-extension
    But we can fix it on our own.  
    Go to your home folder (On Windows you must show hidden files and enter AppData directory) and navigate to subfolder .\App\Inkscape\share\extensions

For version 2.7.1:
    - open file "ebb_serial.py" in text editor and search for the following block:

        EBBport = None
        for port in comPortsList:
            if port[1].startswith("EiBotBoard"):
                EBBport = port[0]   #Success; EBB found by name match.
                break   #stop searching-- we are done.
        if EBBport is None:
            for port in comPortsList:
                if port[2].startswith("USB VID:PID=04D8:FD92"):
                    EBBport = port[0] #Success; EBB found by VID/PID match.
                    break   #stop searching-- we are done.      
 
    - replace "04D8:FD92" with the VID/PID of your Arduino device.  
    - alternatively, you can replace "EBBport = None" with your specific port number:
        EBBport = "COMxx"               #Windows
        EBBport = "/dev/tty[something]" #Linux/Mac

Enjoy :-)

