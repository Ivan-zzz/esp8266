# esp8266
Wemos D1 mini / NodeMCU


BLYNK data transferring + oled 0.96" (i2c) indication
GPS neo-6m module + Wemos D1 mini (ESP8266)
by Ivan-Z `2021

Functionality:
- GPS time and date 
- GPS coordinates (Sattelites number, Lattitude, Longitude, Altitude, Heading, Bearing)
- GPS speedometer
- GPS estimates the range between 2 points: current position and destination point
- Destination point select by button (current position or pre-written into the sketch coordinates)
- Indication on 0.96" oled (i2c)
 

Pin connection:
Wemos D1 mini - GPS neo6m - Oled  - buttons
-----------------------------------------------
5V            - 5v        - 5v    - nc (!!!) dont connect buttons with 5v to wemos D1 mini! its too much! 
3.3v          - nc        - nc    - Buttons 1,2...IN (previously I connected buttons with 5v, so fired-up 2 input ports, now they are always indicate high, working as output only, dont repeat my mistakes! :)))
GND           - GNC       - GND   - GND (with resistor 10k)
D2 (gpio4)sda - nc        - SDA   - nc
D1 (gpio5)scl - nc        - SCK   - nc
D5 (gpio14)   - TX        - nc    - nc
D6 (gpio12)   - RX        - nc    - nc
D0 (gpio16)   - nc        - nc    - button 1 (menu)
D7 (gpio13)   - nc        - nc    - button 2 (select). because of non-working input ports D3/D4 (which I melted by 5v input signal),
                                    its possible to manage all commands by only 1 button, and freeup HSPI 4 pins for sd card, for example.
                                    To avoid using 2nd button, select by short press, and menu change by long press...
                                    usually gpio 12.13.14.15 are used for HSPI connections... so change buttonn pins to D3/D4 if you need SPI.

Long press of select button added. Short press = current point becomes destination. Long press = pre-written into sketch "home" coordinates become a destination point. Gohome mode.


Good luck! ))
