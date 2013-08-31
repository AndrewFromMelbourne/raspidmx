Dispmanx
========

There are a number of APIs available for the Raspberry Pi that can make use
of the computers GPU. These include OpenMAX, Open GL ES(1 and 2) and OpenVG.
The has short examples for theses and other APIs. They can be found in
/opt/vc/src/hello_pi/. Among these examples is a program called
hello_dispmanx. It is a very small example of the Dispmanx windowing
system. Apart from this:-

https://github.com/raspberrypi/firmware/tree/master/opt/vc/src/hello_pi/hello_dispmanx

example, there is very little documentation available for this API.
Hopefully these programs can be used as a starting point.

test_pattern
------------

This test pattern should be familiar to anyone who has used the Raspberry
Pi. It is the same four colour squar displayed when the Raspberry Pi boots.

rgb_triangle
------------

Displays a triangle in a layer with red, green and blue gradients starting
at each corner respectively. Blends to grey in the center. 

life
----

Conway's game of life.

worms
-----

The program raspiworms uses a single 16 or 32 bit RGBA layer to display a
number of coloured worms on the screen of the Raspberry Pi.

pngview
-------

Load a png image file and display it as a Dispmanx layer.

sprite
------

Demonstrates a seamless background image that can be scolled in any
direction. As well as animated sprites.

common
------

Code that may be common to some of the demonstation programs is in this
folder.

building
--------

If you type make in the top level directory, the make file will build all
the different programs in this repository. Each program has it's own make
file, so you can build them individually if you wish.




