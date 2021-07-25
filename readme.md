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

This may be useful if you are starting with battery-powered apps for the atmega328p.  It shows 
one way of doing switch debouncing (there are many), and how to wake the device from sleep.  Unfortunately
my multimeter has only a 200mA scale and I can't measure anything lower than 0.1mA, but since the lamp is
powered by 2xAA eneloop batteries I don't really care about reducing idle consumption below 100uA.  This
arrangement runs idle for at least 3 months.

Sorry, I haven't a circuit diagram, but here's a pic showing how I fitted the board in.  Note that 
I didn't need 4 pins at the end of the 328, it's a nasty thing to do leaving it with its backside sticking 
out like that but I had that bit of stripboard spare and didn't want to use a large piece.  The vertical 
space was too tight to comfortably socket the IC, and I didn't want sharp pins on the bottom so I soldered 
everything to the strip side.  Under the board is some carpet tape.

![unicorn](unicorn.jpg?raw=true)

There is also a youtube video so you can see the light sequences
https://www.youtube.com/watch?v=DPYmhrUM9PU

