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

### Naming Convention

The configurable variables mostly follow a naming convention with the following
groups:

* arena

Configuration items relating to the arena and game play in the arena, for
example the arenas width and height, or how many gladiators are present in the
arena.

* fitness

The configuration items under this namespace are mostly to the weights applied
to various variables which determine the gladiators fitness, they can be zero -
disabling the criteria, negative to punish it, or positive to encourage
behavior that leads to maximizing it.

* draw

Anything under this namespace selects which objects, or how objects, get drawn
in windowed mode. For example the results of the collision detection algorithms
can be drawn to the screen so collisions and intersections can be shown.

* print

When in windowed mode a lot of debugging information can potentially be written
to the screen, however this text takes up a lot of room and only certain items
are really relevant at a time. The variables under this namespace are used to
control what is printed to the screen.

* food

Items under this namespace control aspects about the food objects, for example
its size, how much energy it provides if consumed, if any, and the like.

* gladiator

Configuration items under this namespace control the gladiators themselves, for
example the gladiators size, like with the food, or its speed, or its maximum
field of view.

* projectile

Items related to the projectiles that the gladiators fire come under this
header, items such as energy cost, projectile speed and how damaging a
projectile is.

* program

There are a few unsorted items which do not belong to any group as of yet,
which will also be described.

### Items

* arena\_food\_count

The number of "food" objects that exist in the arena at any one time.

* arena\_gladiator\_count

The number of gladiators in the arena, the minimum value is two.

* arena\_paused

If set true, this pauses the arena, this can be set to true to pause the arena
at startup.

* arena\_random\_gladiator\_start

A boolean describing whether the gladiators start in a random position in the
arena or if they start on the circumference of a circle centered in the middle
of the arena facing outwards.

* arena\_tick\_ms

The amount of time an arena "tick" takes, a tick updates all of the objects;
food, projectiles and gladiators, on the map. It can be set lower than the
framerate but the framerate sets a minimum tick time.

* arena\_wraps\_at\_edges

This controls whether the arena is toroidal or is bounded at the edges.

* brain\_activation\_function

A selector for the [activation function][] used for the neurons in a gladiators
brain.

The values are logistic = 0, hyperbolic tangent = 1, arc tangent = 2, identity
= 3, binary step = 4, rectifier = 5 and sine = 6. 

The activation function used greatly changes how the gladiators evolve,
functions like binary step are particularly poor, the identity function is in
fact even better. The other functions produce minor differences in out comes.

* brain\_input\_normalization\_method

The normalization function used for the inputs, this limits the range of the
inputs.

* brain\_max\_weight\_increment

For the gladiators that are fit enough to make it to the next generation the
brains that control the gladiators undergo mutation, each bias, weight and
threshold for each neuron has a probability of being mutated. But what does a
mutation do? Does it set a weight or bias to a completely new value or does it
modify the old value? In our case we chose to modify the old value by a
continuous [random variable][] in the range [0, 1] multiplied by 
"brain\_max\_weight\_increment". There is then a 50% chance that this value
will be added or subtracted from the current weight, bias or threshold. 

* draw\_gladiator\_collision

If true a red circle will be drawn around each gladiator if two gladiators
collide.

* draw\_gladiator\_target\_lines

If true then the gladiators field of view cone will cone will be partially
drawn to show the angle that the gladiator can see and its direction.

* draw\_gladiator\_wall\_collision

If true, a circle will be drawn around the gladiators when they collide with
the wall, this is of course conditional on *not* being in a toroidal world.

* draw\_line\_circle\_collision

Draw any intersections used for collision detection that use line-circle
collision detection. This draws an object (a small circle) at the point of
the collision.

* draw\_line\_circle\_debug\_line

Draw a line from the origin of the detection to the object being detected for
any line-circle detection. 

* fitness\_weight\_energy

The fitness multiplier for the amount of energy a gladiator has.

* fitness\_weight\_food

The fitness multiplier for the amount of food a gladiator has collected.

* fitness\_weight\_health

The fitness multiplier for the amount of health a gladiator has. 

* fitness\_weight\_hits

The fitness multiplier for the number of hits the gladiator has scored against
its opponents.

* fitness\_weight\_rank

The fitness multiplier for the rank of the gladiator, the rank is the number of
ticks a gladiator survived until it was killed or a new generation was
triggered.

* fitness\_weight\_wall\_time

The fitness multiplier for the gladiator if the wall time counter was exceeded.

* food\_active

Whether the food is active or not. If this is false then the food objects will
not be created in the arena.

* food\_control\_method

The method of control for the food, food can either be set to move in a random
direction each tick or move with an initial random direction and bounce off
walls. (0 = random, 1 = bounce).

* food\_distance\_per\_tick

The distance the "food" can travel, if any, in a tick.

* food\_health

The amount of health the food replenishes if eaten by a gladiator.

* food\_nourishment

The amount of energy that eating food provides.

* food\_respawns

Whether the food respawns if eaten.

* food\_size

The size of the food in "units". This is relative to the size of arena
(specified by *Xmin*, *Xmax*, *Ymin* and *Ymax*).

* gladiator\_bounce\_off\_walls

Whether the gladiator bounces off walls or not.

* gladiator\_brain\_depth

The number of layers in a gladiators neural network.

* gladiator\_brain\_length

The length of each layer in a gladiators neural network.

* gladiator\_distance\_per\_tick

The distance in "units" that a gladiator can travel per tick.

* gladiator\_energy\_increment

The increment in energy every tick that the gladiator receives.

* gladiator\_field\_of\_view\_divisor

The gladiator can change its field of view by two outputs, open and close,
however the amount it can change by each tick is limited to maximum field of
view angle for a gladiator divided by this divisor.

* gladiator\_fire\_threshold

The threshold on the output value a gladiator needs to trigger a firing event,
which will fire a projectile if the gladiator has enough energy and the refire
rate cool down period has expired.

* gladiator\_fire\_timeout

The minimum time it takes after a gladiator has fired before the gladiator can
fire again. The time is in ticks.

* gladiator\_health

The health of the gladiator. When this value drops below zero the gladiator
will die and become inactive. 

* gladiator\_max\_energy

The maximum energy a gladiator starts with.

* gladiator\_max\_field\_of\_view

The maximum gladiator field of view angle, in radians.

* gladiator\_min\_field\_of\_view

The minimum gladiator field of view angle, in radians.

* gladiator\_size

The size of the gladiator in "units". This is relative to the size of arena
(specified by *Xmin*, *Xmax*, *Ymin* and *Ymax*).

* gladiator\_starting\_energy

The amount of energy a gladiator starts with.

* gladiator\_turn\_rate\_divisor

A gladiator can turn left or right one radian per tick divided by this amount
as a maximum.

* gladiator\_vision

A global variable containing the current gladiator field of view angle in
radians.

* gladiator\_wall\_time

The amount of time a gladiator can spend in contact with one of the arena walls
before its fitness is affected, the idea is to make gladiators that get stuck
on the walls less fit, which of course can be configured by the fitness
multipliers.

* max\_ticks\_per\_generation

The number of ticks a generation takes, as a maximum. If one of the gladiators
kills all of the other gladiators then the generation will end early, if not it
will continue the tick counter expires by going over this value.

* mutation\_rate

The mutation rate, which will be divided by the depth multiplied by the length
of the neural network used. This is used to mutate the biases and the weights
in a gladiators "brain".

* use\_crossover

Whether to use cross over when selecting the next generation of gladiators,
this breeds a new gladiator from the best and the second best gladiator to go
into the population for the next round.

* print\_arena\_tick

Print the current tick count for the arena onto the display, if the display is
present.

* print\_fps

Print the FPS counter for the program onto the display, if the display is
present.

* print\_generation

Print the current generation to the display, if the display is present.

* print\_gladiator\_energy

For each gladiator print its current energy count, if the display is present.

* print\_gladiator\_fitness

For each gladiator print its current fitness level, if the display is present.

* print\_gladiator\_health

For each gladiator print its current health, if the display is present.

* print\_gladiator\_hits

For each gladiator print the number of hits it scored, if the display is present.

* print\_gladiator\_inputs

For each gladiator print the gladiators input values, if the display is present.

* print\_gladiator\_mutations

For each gladiator print the number of mutations it underwent in this
generation.

* print\_gladiator\_orientation

For each gladiator print the gladiators orientation in radians, if the 
display is present.

* print\_gladiator\_outputs

For each gladiator print the gladiators output values, if the display is present.

* print\_gladiators\_alive

For each gladiator print whether the gladiator is still alive, if the display is present.

* print\_gladiator\_state1

For each gladiator print the gladiators internal state variables, if the display is present.

These state variables are part of an experiment to see if the network can learn
to use memory.

* print\_gladiator\_team\_number

For each gladiator print whether the gladiator team number, if the display is
present.

* print\_gladiator\_total\_mutations

For each gladiator print the number of mutations it underwent, if the 
display is present. This value is not completely accurate if the crossover
mechanism is present.

* print\_gladiator\_x

For each gladiator print whether the gladiators x position, if the display is present.

* print\_gladiator\_y

For each gladiator print whether the gladiators x position, if the display is present.

* program\_headless\_loops
* program\_log\_level

The global log level for this program, this affects functions "error",
"warning", "note", and "debug". If the log level is too low the functions will
not print out their error message, however any other side affects that the
logging function might have will continue to take place (for example, the
"error" function will kill the application if it is called, but will optionally
print out an error message).

The levels are (all messages off = 0, print errors = 1, print warnings = 2,
print notes = 3, print debugging messages = 4, print all messages = 5). If the
log level is "3", all messages classes with a lower or equal log level to
"note" will be printed.

* program\_pause\_after\_new\_generation

If true, pause the program after a new generation has been generated. This of
course does not affect headless mode.

* program\_random\_seed

The seed to provide to the Pseudo Random Number Generator, this program does
not use the standard library provided PRNG to ensure that the source of the
"random" numbers is repeatable across platforms. If set to zero the PRNG will
be seeded with the time instead.

* program\_run\_headless

If true, this program will run in "headless mode", the gladiators will battle
but this will not be displayed with in window, textual information about the
gladiators may be displayed.

* program\_run\_window\_after\_headless

If running in headless mode and the number of loops has selected by
"program\_headless\_loops" has been complete there are two possible options,
either quit the program, possibly after printing the results, or create a
window and continue on with the tournament but now the tournament will be drawn
on the screen. If true, the later option will happen, if false, the former.

* projectile\_damage

The amount of damage a projectile inflicts.

* projectile\_distance\_per\_tick

The distance a projectile travels in a single tick.

* projectile\_energy\_cost

The amount of energy required to fire a projectile.

* projectile\_range

The range a projectile can travel before becoming inactive.

* projectile\_size

The size of a projectile in "units".

* window\_height

The height of the window, the window should be adjustable, this is just the
starting height.

* window\_width

The width of the window, the window should be adjustable, this is just the
starting width.

* window\_x\_starting\_position

Starting position of the window, "x" or horizontal coordinate. This affects
where the windowed application is placed on the screen. 

* window\_y\_starting\_position

Starting position of the window, "y" or vertical coordinate. This affects
where the windowed application is placed on the screen.

* Xmax

Maximum "x" value for the arena, objects exceeded this value will either be
limited or wrapped around to "Xmin".

* Xmin

Minimum "x" value for the arena, objects exceeded this value will either be
limited or wrapped around to "Xmax".

* Ymax

Maximum "y" value for the arena, objects exceeded this value will either be
limited or wrapped around to "Ymin".

* Ymin

Minimum "y" value for the arena, objects exceeded this value will either be
limited or wrapped around to "Ymax".

## References

This project will be based on this one:

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
* Different types of AI brains could be loaded, for example the best evolved
neural network AI could go up against a manually programmed one, using
[GPGOAL][] as the engine. (Also see
<http://alumni.media.mit.edu/~jorkin/goap.html>)

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

