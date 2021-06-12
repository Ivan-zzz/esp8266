# esp8266 Wemos D1 mini
BLYNK data transferring + oled 0.96" (i2c) indication
GPS neo-6m module + Wemos D1 mini (ESP8266) + 5883 compass
by Ivan-Z `2021

Functionality:
- GPS time and date 
- GPS coordinates (Sattelites number, Lattitude, Longitude, Altitude, Heading, Bearing)
- GPS speedometer
- GPS estimates the range between 2 points: current position and destination point
- Destination point select by button (current position or pre-written into the sketch coordinates)
- Indication on 0.96" oled (i2c)
- Connection to the internet (data transferring to Blynk server/vidget). 
- 5883 magnet compass module added, indication: gauge + bearing. (i2c)
 
Pin connection:

Wemos D1 mini - GPS neo6m - Oled  - buttons                 - 5883 compass

5V            - 5v        - 5v    - nc                      - nc

3.3v          - nc        - nc    - Buttons 1,2...IN        - Vin

GND           - GNC       - GND   - GND (with resistor 10k) - GND

D2 (gpio4)sda - nc        - SDA   - nc                      - SDA (both 5883 + Oled on the same wires)

D1 (gpio5)scl - nc        - SCK   - nc                      - SCL (both 5883 + Oled on the same wires)

D5 (gpio14)   - TX        - nc    - nc                      - nc

D6 (gpio12)   - RX        - nc    - nc                      - nc

D0 (gpio16)   - nc        - nc    - button 1 (menu)         - nc

D7 (gpio13)   - nc        - nc    - button 2 (select)       - nc.


Long press of select button added. Short press = current point becomes destination. 
Long press = pre-written into sketch "home" coordinates become a destination point. Gohome mode.

Good luck! ))
