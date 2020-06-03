# readme.md
# Gladiators

An arena for battling [neural network][] controlled gladiators and improving
them with [genetic algorithms][].

Below is a picture of some of the game play:

![gladiators-1][]

## Introduction

This program is intended to be a simple simulator for a battle arena where
'gladiators' (simple [neural network][] controlled game artificial intelligences) 
compete, this arena is combined with genetic algorithms to improve the AI. This
is a toy project.

The idea is to get the program to a stage where it can evolve a gladiator that
can move around, hunt for food, kill enemy gladiators and doge projectiles.

Many of the parameter are configurable, so the food game mechanic can be
disabled for example, this changes the direction in which the gladiators can
evolve. A few interesting gladiators have been produced by this tool, some 
which can dodge slightly, and seek out other gladiators, with the right 
configuration.

This program was inspired by watching [A video][] of a program which does
roughly the same as this one, with an explanation (in French, at 
[doublezoom.free.fr][]) on how the program shown in the video works. Whilst 
most of the general details are explained there does not seem to be any 
source code for this application, which is a shame, the video shows the
gladiators learning how to move, how to fire, and eventually how to dodge enemy
projectiles.

## Building

To build [gcc][] (or a suitable C compiler), [make][] (GNU Make was used), 
and [OpenGl][]/[Glut][] (pre-compiled binaries are available for most Linux
distributions as well as Windows) are needed. Type 'make' to build and 'make
run' to run the default configuration, this should take a while to run and then
pop up a window with some 'evolved' gladiators.

## Architecture and Theory

As this project was meant to be an introduction for the author into both
[OpenGl][] and [neural networks][] the theory will be brief and light. For a
good introduction to [neural networks][] see the [Usenet FAQ][].

### Game mechanics

### Neural Network

The neural network is quite configurable, there is nothing special about it
however, the number of layers in the neural network can be configured as can
the size of each layer, although the length of the layers much be equal for
each layer, and must be greater or equal to the number of inputs or output,
which ever is larger. Each neuron is fully connected to the outputs of the
previous layer.

This Artificial Neural Network (ANN) forms the "brain" of the gladiator, this
ANN is fed inputs about the gladiator and its environment and has a few 
outputs it can actuate, such as whether to turn left or to turn right.

The [activation function][] for the ANN is configurable, some work better than
others and the difference can be quite substantial. It helps if the function is
[smooth][], 

### Fitness and selection

### Mutation and Cross Over

## Running the program

The program has two modes of operation, a headless mode which runs a lot
quicker and does not draw the arena and gladiators, and a mode which uses
[Glut][] and [OpenGL][] to display the gladiators competing against each other.

The techniques to draw the objects is quite primitive, however it was partly
meant as an introduction for me in how to use [OpenGL][]. 

A file called **gladiator.conf** will be loaded at startup, if present, any
command line options which affect configurable items (such as increasing the
log level) will take precedence over the configuration items.

## Configurable Items

There are many configurable items within the program that affect the outcomes
and strategies that the gladiators must adopt to succeed.

The default values for these items will not be covered here, but can be viewed
in header file [vars.h][] along with a boolean indicating whether the value is
allowed to be zero.

## References

This project is based on this one:

* <https://www.youtube.com/watch?v=u2t77mQmJiY>
* <http://doublezoom.free.fr/programmation/AG_Exemple_Fighting.php>

For collision detection:

* <http://elancev.name/oliver/2D%20polygon.htm>

For neural networks:

* <ftp://ftp.sas.com/pub/neural/FAQ.html>
* <https://en.wikipedia.org/wiki/Activation_function>

## Future direction

It would be good to see if it would be possible to evolve gladiators that could
cooperate together in teams against other teams. 

Another possible avenue for development would be to make this more into a game
where a player could compete against the results of the contest to see which
one can last the longest.

### To Do

* The physics engine is a big dodgy and could use work
* Some of the configuration options provide quite nice little gladiators that
sort of follow each other around and fire, it would be good to provide
examples.
* Some of the inputs are discontinuous and are thus probably not very useful,
this could be changed.
* Work on S-Expression library so state can be saved and loaded in again, this
should be turned into a standalone library.
* Work on adding a player: They should only be able to see what a gladiator
can, which is quite limiting, adding to the challenge.
* The selection mechanism needs to be improved, each generation should have a
much higher population count, and the selection mechanisms needs to be
changed. A population of about 50 gladiators would fight in a tournament, with
greater probability of reproduction going to the winners.
* Completely separate out GUI so, so a headless, non-gui executable can be
produced. Perhaps a better looking SDL version could be produced as well.
* Performance improvements so evolution can take place quicker
* Different types of AI brains could be loaded, for example the best evolved
neural network AI could go up against a manually programmed one, using
[GPGOAL][] as the engine. (Also see
<http://alumni.media.mit.edu/~jorkin/goap.html>)
* A version in which the gladiators would have to learn to pick up food and
drop it off at a place could be made as well. This would probably require two
more inputs; food carried and drop off point in vision.

[GPGOAP]: https://github.com/stolk/GPGOAP
[A video]: https://www.youtube.com/watch?v=u2t77mQmJiY
[doublezoom.free.fr]: http://doublezoom.free.fr/programmation/AG_Exemple_Fighting.php
[Glut]: https://www.opengl.org/resources/libraries/glut/
[OpenGL]: https://www.opengl.org/
[neural network]: https://en.wikipedia.org/wiki/Artificial_neural_network
[neural networks]: https://en.wikipedia.org/wiki/Artificial_neural_network
[activation function]: https://en.wikipedia.org/wiki/Activation_function
[random variable]: http://www.stat.yale.edu/Courses/1997-98/101/ranvar.htm
[Circular Sector]: https://en.wikipedia.org/wiki/Circular_sector
[genetic algorithms]: https://en.wikipedia.org/wiki/Genetic_algorithm
[Usenet FAQ]: ftp://ftp.sas.com/pub/neural/FAQ.html
[gladiators-1]: img/gladiators-1.png
[smooth]: https://en.wikipedia.org/wiki/Smoothness
[gcc]: https://gcc.gnu.org/
[make]: https://www.gnu.org/software/make/
[glut]: http://freeglut.sourceforge.net/

<style type="text/css">body{margin:40px auto;max-width:850px;line-height:1.6;font-size:16px;color:#444;padding:0 10px}h1,h2,h3{line-height:1.2}</style>

