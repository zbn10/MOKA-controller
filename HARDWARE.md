Hardware Release Notes
2025/5/16 MKBv0.9, global label v00.09.00

2025/6/19 MOKAv1, global label v01.00.00
- Supported main MCU: XIAO RP2350 only not RP2040.
- LEDs Numlock, Capslock, Scrlock removed because they are rarely used.
- Change the load switch from CH217 to TCK107AF resulting active low to high.
- Joystick removed.
- Touchpad space added.
- Analog photoIC sensor and infrared sensor space added on left.
Known issues
(- Serial debug RX TX silkscreen are swapped but firmware swap lines to match the printing.)

2025/9/27 MOKAv2.0, global label v02.00.00
- for develop version, not released
- reposition key switches
- change LED component from WS2812C-2020 to SK6812E-mini
- Change the operating voltage of the MCUs and mag sensors from 3.3V to 5V to match the logic voltage level of the LEDs,
- replace 0.1u decoupling capacitors with 1u for mag sensors and LEDs
- change 0.1uF capasitor pad size from 2102 to 1608
- change capacity of sensor bypass capacitors from 0.1uF to 1uF
- attiny bypass capacitors into one
- add logic level shifter between 3.3V I2C and 5V I2C
- support waveshare RP2xx0-zero newly
- SEEED XIAO RP2xx0 is no longer supported
- change I2C0 GPIO from GP28/29 to GP8/9
- move load sw, PhotoIC, Infrared Sensor, tact sw, LED to the center
- move OLED Display to the right hand PCB
- move UPDI components to the left hand PCB
- change left and right connection cable from USB to JST SH
- add Touchpad UPDI Socket
- expand Touchpad size
- support Touchpad v0, 3.3V version
- add debug ports near JST SH connectors
- change the MCUs numbering from [0 1 2] [5 4 3] to [0 1 2] [3 4 5], and reverse mag sensors numbering on right side.
- revert the debug serial tx rx line. fix silkscreen. v1.0.0 has silkscreen cosmetic bug.

2025/10/23 MOKAv2.1, global label v02.01.00
- Touchpad ATTiny1616 drive voltage changed 3.3v -> 5v to prevent 5v UPDI wirtes in 3.3v state
- Place I2C level converter BSS138 next to ATTiny1616
- Remove right side slide switch using to change NORMAL <-> UPDI voltage
- Remove left side 3.3v load switch
- Reposition UPDI pin sockets
- Reposition UPDI USB connector and move to back
- Reposition Touchpad pin positions
- support Touchpad v1, 5V version
- change right side 3.3v linear regulator part to smaller one
- making SK6812 LED holes smaller and move up a little
- Remove system LED of SK6812. its not driven by 3.3v logic pin
- use waveshare RP2350 built in WS2812 for system LED

2025/x/x MOKAv2.1.1, global label v02.01.01
- improve silkscreen visibility of 3.3v linear regulator
- change silkscreen R & C value to three digit notation
