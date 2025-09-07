CURRENT MEASUREMENTS

PERIPHERALS:

R503 FPM:

When VCC is connected to a 3.31V supply, FPM draws 13.8mA of quiescent current. When
VCC is disconnected (3.3VT connected to 3.31V supply), it draws 2.9uA. 

FS5106B high-torque servo:

One of them draws 6.1mA when connected and stabilizes at 4.6mA. The other drew
variable amounts initially (4.7mA, 5mA+, but mostly 4.7). Both draw 4.6mA of
quiescent current with a 5.07V supply.

TARGET CURRENT DRAW

 1. 2.7mA for 1 month
 2. 1.4mA for 2 months
 3. 900uA for 3 months
 3. 694uA for 4 months

LINEAR REGULATORS

 1. When the ATmega328P is in normal mode, the MCU + FPM + Servo + linear
    regulators draw 30.6mA of quiescent current at 5.07V.
 2. When the ATmega328P is in power down mode, the MCU + FPM + Servo + linear
    regulators draw 26.2mA of quiescent current at 5.07V.
 3. When the ATmega328P is in power down mode, the MCU + linear regulators draw
    13.7mA of quiescent current at 5.07V (without MOSFETS).

MP1584EN BUCK CONVERTER

 1. When the system is in power down mode, the MCU + FPM + Servo draw 8.9mA at
    3.3V. The real amount is likely about 8.9 + 4.6 = 13.5mA.
 2. When FETs are used quiescent current drops to 0uA.
 3. Active current draw (when the servo rotates) measured are 152.8mA, 162.8mA,
    170.3mA

REMARKS

 1. Can't use 3.3V FPM modules with Arduino Uno. The RX and TX lines have
    resistors. 
 2. When measuring current with the buck chips on, the servo doesn't respond.
    So, the value measured probably doesn't include the current the servo 
    draws.
 3. N-channel high-side switching doesn't work without a 9V supply. 7.2V+ with 
    batteries don't work either.
