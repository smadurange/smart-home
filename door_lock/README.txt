ELECTRICAL CONNECTIONS

Following connections puts the atmega328p in a configuration that mirrors the 
configuration of an Arduino Uno.

  1. Pin 1 of the IC connects to the 5V via a 10k resistor to prevent the 
     atmega328p from resetting.
  2. A 16MHz crystal oscillator connects to pins 9 and 10, and each pin of the 
     crystal oscillator connects to ground via 22pF capacitors.
  3. Pins 7, 20 and 21 (counted clockwise from the bottom left of the IC) 
     connect to 5V.
  4. Pins 8 and 22 connect to ground.

Now connect the PWM line of the servo to pin 15 of the IC.
