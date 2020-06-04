% arena(1) | Evolve Neural Net Controlled Gladiator

# NAME

ARENA - A program for experimenting and evolving neural net controlled gladiators

# SYNOPSES

arena [-] [-h] [-v] [-s] [-p] [-H]

# DESCRIPTION

	Author:     Richard James Howe
	License:    MIT
	Repository: <https://github.com/howerj/gladiators>
	Email:      howe.r.j.89@gmail.com
	Copyright:  2016-2020 Richard James Howe

This is a simple toy program designed to display a series of 'gladiators'
that can fire at and evade each other. The gladiators are controlled by a
neural network, which gets mutated and bred every generation of gladiators.
Which gladiators make it into the next round depends on their fitness level
determined at the end of the current round.

There is a default configuration file called "gladiators.conf", which can be
regenerated if it is missing. This file will be loaded, if present, after any
command line arguments have been processed.

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

# LICENSE

This is released under the MIT License.

# OPTIONS

Several options may be given to the program before it becomes a Windowed
application.

- '-' 

Stop processing any command line options

- '-h' 

Print out a help message and exit with an error.

- '-v;

Increase the verbosity or log level of the program.

- '-s'

Attempt to save a default "gladiators.conf" configuration file to disk, and
exit.

- '-p'

Print the default configuration to stdout(3) and exit.

- '-H'

Run in 'headerless' mode, or without a GUI.

# EXAMPLES

	./arena

Run the arena, loading the default configuration file if present from the disk.


# EXIT STATUS

This command returns zero on success and non zero on failure.

# BUGS
This program is a work in progress, the gladiators do not do anything useful at
the moment.


## BUILDING

To build [gcc][] (or a suitable C compiler), [make][] (GNU Make was used), 
and [OpenGl][]/[glut][] (pre-compiled binaries are available for most Linux
distributions as well as Windows) are needed. Type 'make' to build and 'make
run' to run the default configuration, this should take a while to run and then
pop up a window with some 'evolved' gladiators.

## Architecture and Theory

As this project was meant to be an introduction for the author into both
[OpenGl][] and [neural networks][] the theory will be brief and light. For a
good introduction to [neural networks][] see the [Usenet FAQ][].

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

# Future direction

It would be good to see if it would be possible to evolve gladiators that could
cooperate together in teams against other teams. 

Another possible avenue for development would be to make this more into a game
where a player could compete against the results of the contest to see which
one can last the longest.

# To Do

* The physics engine is a big dodgy and could use work
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
* Generate the manual page from this file
* Make the program into a library with a CLI and GUI front end

[GPGOAP]: https://github.com/stolk/GPGOAP
[A video]: https://www.youtube.com/watch?v=u2t77mQmJiY
[doublezoom.free.fr]: http://doublezoom.free.fr/programmation/AG_Exemple_Fighting.php
[OpenGL]: https://www.opengl.org/
[neural network]: https://en.wikipedia.org/wiki/Artificial_neural_network
[neural networks]: https://en.wikipedia.org/wiki/Artificial_neural_network
[activation function]: https://en.wikipedia.org/wiki/Activation_function
[random variable]: http://www.stat.yale.edu/Courses/1997-98/101/ranvar.htm
[Circular Sector]: https://en.wikipedia.org/wiki/Circular_sector
[genetic algorithms]: https://en.wikipedia.org/wiki/Genetic_algorithm
[Usenet FAQ]: ftp://ftp.sas.com/pub/neural/FAQ.html
[smooth]: https://en.wikipedia.org/wiki/Smoothness
[gcc]: https://gcc.gnu.org/
[make]: https://www.gnu.org/software/make/
[glut]: http://freeglut.sourceforge.net/

<style type="text/css">body{margin:40px auto;max-width:850px;line-height:1.6;font-size:16px;color:#444;padding:0 10px}h1,h2,h3{line-height:1.2}</style>

