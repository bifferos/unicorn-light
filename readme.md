A small project to alter the unicorn lights you can find on amazon to add some special effects, and push-on push-off.
Uses an atmeg328p with internal 8MHz oscillator.

Required:
1 x Unicorn light with 11 bulbs
USBASP programmer:  https://www.ebay.co.uk/itm/322662323277
avr-gcc (from Debian Buster packages)
avrdude (Debian Buster)
I'm using atmega328p chip DIP package
0.1uF cap for supply decoupling.
Wire and connectors.

This may be useful if you are starting with the atmega328p, it was a learning project for me.  It shows 
one way of doing switch debouncing (there are many), and how to wake the device from sleep.  Unfortunately
my multimeter has only a 200mA scale and I can't measure anything lower than 0.1mA, but since the lamp is
powered by 2xAA eneloop batteries I don't really care about reducing idle consumption below 100uA.
