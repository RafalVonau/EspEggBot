# EggBot on ESP8266

## Info

This repository contains the software for my ESP8266 based EggBot.
An [EggBot](https://egg-bot.com/) is a machine for drawing on eggs. I built an eggbot from 3d printed parts from the [thingiverse](https://www.thingiverse.com/thing:3431363) website. 
I'm using an D1 mini(ESP8266) for controlling the steppers and the servo. 

The project is based on:
- [EggDuino](https://github.com/schlion/EggDuino) - some code ported to ESP8266
- [EggBot Inkscape plugin](http://wiki.evilmadscientist.com/Installing_software) - I modified the plugin to automatically detect the USB to UART converter present on the D1 mini board and in case it fails to detect the USB converter, try to connect via TCP with eggbot.local:2500 address
- [EggEsp](https://github.com/M4GNV5/EggEsp) - svg to gcode parser
- [SvgToGCode](https://github.com/evomotors/SvgToGCode) - svg to gcode parser

Features:
- Implemented Eggbot-Protocol-Version 2.1.0
- Turn-on homing: switch-on position of pen will be taken as reference point.
- No collision-detection!!
- Supported Servos: standard analog mini servos like TG9e, SG90, ES08MA, HXT900, etc.
- Taking svgs directly (over www) -> no need for external software.
- Inkscape plugin (commands over serial port or TCP socket on port 2500).

The test project was built on D1 mini board connected to PC computer over USB as /dev/ttyUSB0 under Linux.
Uncomment and modify Wifi client settings in secrets.h file:
* #define WIFI_SSID                "EggBot"
* #define WIFI_PASS                "EggBotPass"

# Parts needed:
* D1 mini (ESP8266),
* DC/DC converter 12V to 5V,
* DC/DC converter 5V to 3.3V,
* 2 x TMC2208/A4988/equivalent,
* 2 x Stepper mottor Nema 17
* Servo SG90/equivalent
* 3D printed and mechanical parts from [thingiverse](https://www.thingiverse.com/thing:3431363)
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

![alt tag](https://github.com/BubuHub/EspEggBot/blob/master/blob/assets/schematic.png)

## Building

The project uses platformio build environment. 
[PlatformIO](https://platformio.org/) - Professional collaborative platform for embedded development.

* install PlatformIO
* enter project directory (esp8266_firmware)
* connect Webmos D1 mini board to PC computer over USB cable.
* type in terminal:
  platformio run -t upload

You can also use IDE to build this project on Linux/Windows/Mac. My fvorite ones:
* [Code](https://code.visualstudio.com/) 
* [Atom](https://atom.io/)

## Inkscape plugin installation

Installation:
1. Install inkscape version 0.94
2. Copy inkscape_plugin subdirectories to inkscape user folder:
* On Linux   (~/.config/inkscape/),
* On Windows (c:\Users\[UserName]\AppData\Roaming\inkscape\)

Enjoy :-)

