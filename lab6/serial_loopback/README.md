Serial Loopback Test
====================

This example shows how to create an additional serial port on the Heltec board
to read and write data over serial.

All this does is a simple loopback test: the board sends data on a TX pin and
then receives it with its own RX pin.

Setup
-----

To use this example, you need to jumper (connect) the pins labeled 21 and 26.

You should then see:

```
RECV: A
RECV: A
RECV: A
RECV: A
```

print in the terminal.

Pin Mapping
-----------

- TX pin: 26
- RX pin: 21

