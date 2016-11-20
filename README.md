The following contains a small bit of code for simulating the evolution of wind-blown sand dunes,
by means of the algorithm specified in
   Werner, B. T. ``Eolian dunes: computer simulations and attractor interpretation.''
   Geology 23, no. 12 (1995): 1107-1110.

As an example, see the formation of a transverse dune (represented as a heightfield) at
http://htmlpreview.github.io/?https://github.com/kilimanjaro/dunes/blob/master/transverse_dune.html

*****************

The code is split between two files. The simulation code is in werner.cpp,
and some helper code for generating animations is in werner.py

To build, type `make werner'.

To run, type `werner <n> <infile> <outfile>'. Here <n> denotes the number of iterations
to simulate, <infile> is a slabfield specification, and <outfile> is where the resulting slabfield
is stored.

Example usage:
	`werner 1000 square_500.txt square_500_2.txt'
results in 1000 iterations of the werner algorithm, beginning with the slabfield stored in square_500.txt,
and saving the final result in square_500_2.txt

Intermediate snapshots are saved in a temp directory, the path of which is displayed
when the simulation terminates. These may be converted to a video by the method
     `make_video(tmpdir,vname)'
in werner.py, where tmpdir is a string with the path to the temp directory, and vname
is the file name which the video is saved. As an example, see the video `square_500.html'.


*****************
Slabfield format

The basic object being manipulated is a `slabfield', which which is simply a n x m array
of slab heights, as specified in the Werner paper. A slabfield specification is a text file with
the following format:

<width> <height>
<row>
...
<row>

where <width>,<height> are integers specifying the dimension of the slabfield. There are <height>
<row>'s in the file, each of which consists of <width> integers, separated by spaces.

See, for example, square_500.txt
