# Nelma-cap

Numerical calculation of capacitance in print
circuit

Author: Tomaž Šolc

Seminar paper in the subject of electromagnetics

Ljubljana, September 2006

(Translated from Slovene with help of Google Translate)

## Index

- Introduction
  - Motivation
  - Choice of capacitance calculation method
- Methods
  - Electric field and capacitance
  - Finite difference method
  - SOR iteration method
- Implementation
  - Using the program
  - Description of circuit geometry
- Results
  - Comparison with analytical calculations
  - Symmetry test
- Conclusion
- Literature

## Introduction

### Motivation

When designing electronic circuits for low and medium frequency signals,
we assume that the mechanical dimensions of the circuit are small in comparison with
wavelengths of electromagnetic waves generated by signals
in the circuit. This assumption allows us to process electronic components
are given as concentrated (point) elements, and the influence of the connections between them
let's ignore.

When the frequencies of the signals in the circuit increase, however, certain connections influence
we can no longer ignore it. Due to the geometry of ordinary circuits (i.e. circuits on
printed matter), the first adverse effect is usually the shaken capacitance
between the connections in the circuit. These cause the signal lines to contact the ground
and increase eavesdropping between individual signal lines. Together with the
delayed capacitances also cause delayed capacitances.
in the transmission of signals. At even higher frequencies, the signals come to the fore
still shaken inductors and then the connections in the circuit must be treated as
electromagnetic lines thus ending the simplification for concentrated circuits.

Since the treatment of circuits with lines is much more demanding than the treatment of concentrated circuits, it is obvious that we want to stick to these simplifications for as long as possible.
vitev. Shaken capacitances can be incorporated into the model relatively easily
concentrated circuits (approximately they can be added to the circuit as concentrated
elements), so this is the simplest way to stick to simplification as well
even slightly across the border when they would otherwise no longer apply. Of course, this requires to be shaken
capacitances are known, as this is the only way to take them into account in planning
circuit.

### Choice of capacitance calculation method

The printed circuit board is basically a three-dimensional structure consisting of conducting bodies
and dielectrics. The only property of geometry that can help us with
calculation, is that the printed circuit board has a layered structure: along one geometric axis ga
can be divided into layers in which the material is homogeneous along this axis. Large-
times we also encounter layers that are homogeneous along the other two axes (na
example a printed circuit substrate that has no connections between conductive layers).

The connections on the printed circuit board are in the vast majority of cases irregular,
therefore, any analytical treatment of capacitance is omitted. Thus, we are left with only numerical methods.
Several numerical approaches have been described in the literature:

For a completely general calculation of capacitance between conducting bodies,
surrounded by dielectrics, we can use or methods that focus
on the calculation of the electric field (finite difference method and finite difference method)
elements) or methods that focus on calculating the distribution of
color on translation bodies. The latter is characterized by the fact that the computational
increases with the complexity of the geometry of conducting and dielectric bodies,
which we are dealing with. On the other hand, the computational complexity of the first methods
increases with the size of the space we are considering.

However, there are also methods that focus on the aforementioned specific
the geometry of the printed circuit board, and which allow simplified computation.
The most interesting of these is DRT (Dimension Reduction Technique) [1], which
allows to calculate the field for each layer of the circuit separately. On the other hand
such methods mostly require that the circuit can be divided into independent ones
units also along the other two coordinate axes [2].

Because the printed circuit board contains a rather complex geometry for its size,
in my opinion, methods that focus on computing take precedence
fields. These allow better computational efficiency in conventional printed circuit boards
powers. On the other hand, these methods still require significant amounts
computer memory. In this respect, the finite difference method has
significant advantage over the finite element method. Evenly distributed
A network of points can be stored in memory much more efficiently than
unevenly distributed boundary points in the finite element method.

Advanced methods such as DRT otherwise promise even better computational efficiency
power, but in my experience to calculate capacitance in print
circuits are not the most suitable. There are mostly no printed circuit boards
can be divided into independent units, as is possible with integrated ones
circuits (for which DRT is primarily intended).

For these reasons, I have decided to use the todo finite differences.
It will later turn out that in addition to the above.
The advantage is that the computer algorithm can be significantly optimized,
considering the layering of the printed circuit board. At the same time, it is an advantage
also the considerable simplicity of the method, which favorably affects the number of errors at
programming.

## Methods

The following is a more detailed description of the capacitance calculation methods used.

Hereinafter, the parts of the conductor are the nodes of the circuit between which
we calculate the capacitances.

### Electric field and capacitance

The basic principle of capacitance calculation that I used is the following:

Place all conductive and non-conductive parts of the printed circuit board in the room.
Connect a voltage of 1V to the i-th conductive part, and ground all other parts.
We also place a grounded conductive box in the space and thus limit ourselves to
the final volume in which we will calculate the electric field.

The size of the grounded box is determined so that it covers the entire part of the conductor
under voltage and yet certain of its surroundings. The size of the surroundings
sna calculation accuracy and computational complexity (larger surroundings mean larger
the volume in which we need to calculate the field). With the size of a grounded box
basically we say how far away two more grounded objects can be yes
the capacitance between them can be assumed to be zero.

In the space inside the box, the Laplace differential equation now holds for
potential (no free charge carriers in the room):

*** formula (1) ***

Based on this equation and the boundary conditions we determined with the box and
conducting bodies at known potentials, we can calculate the potential in all
points of space. From the potential value we can then calculate the field after
equations:

*** formula (2) ***

When we know the electric field in space, we can according to Gauss's theorem
we calculate the charge on the j-th conductor by integrating the field over the plane
A embracing the selected conductor:

*** formula (3) ***

We can now calculate the shaken capacitances between the conductor i and
translator j by definition:

*** formula (4) ***

When we repeat the above procedure for all i and j, we calculated all the capacitive
sti in the circuit. In fact, without much increase in computational complexity,
each capacitance is calculated twice (C i, j = C j, i), since the calculation is
is by far the most demanding step, and the additional calculation of integrals is not affected
essentially at the time of calculation. On the other hand, with this additional calculation
we get a way to estimate the calculation error. If Ci,j is quite different from
Cj,i, then our calculation was not accurate enough.

### Finite difference method

The finite difference method allows us to solve the above-mentioned La-
the plots of the differential equation with boundary conditions are translated into the solution of the system
linear equations ([3], pages 80-88).

We do this by placing a grid in the computing space. Values ​​according to
potential is then calculated only at the nodes of the network. Material (conductor or
dielectric) should be homogeneous within one section of the network. In my pri-
I have simplified the calculation to the extent that the step of the grid is for each dimension
particularly constant (these steps are hereinafter denoted by hx, hy, and hz).

The general equation in the system of linear equations into which
Laplace's equation now looks like this:

*** place a figure here ***

Where it applies

*** place a figure here ***

*** place a figure here ***

Figure 1: Interpretation of indices at V and ε.

Boundary conditions also introduce equations into the system in the form:

*** place a figure here ***

The general equation mentioned above is really only needed at the boundaries between
two layers. Within the layer, the substance is homogeneous along the axis, which means that
the equation is simplified as follows:

*** place a figure here ***

Where it applies

*** place a figure here ***

For layers that are also homogeneous along the x and y axes, we use the equation:

*** place a figure here ***

### SOR iteration method

The number of unknowns and thus the number of equations in the system obtained with the help
finite differences, there are many - it is equal to the number of nodes in the network, which is growing
with the third power of the dimensions of the circuit under consideration. Even smaller circuits bring
systems with 10^6 unknowns. For larger circuits or denser computing networks
we also quickly reach 10^9 unknowns, thus already reaching the limits of performance
working memory of personal computers.

Obviously, non-iterative methods fall away for solving such large ones
systems. All these methods require a complete record of the equations that occur
in the system, and this requires a square-dependent memory capacity
the number of variables in the system.

Of the available iteration methods, I decided to use the SOR iteration
for the following reasons:

- It is relatively simple, which again has a positive effect on the number of errors in
programming.

- Convergence is ensured as the system of equations is diagonally
ten.

- With the appropriate choice of the parameter, ω ensures fast convergence.

- Unlike some other methods it does not require it in memory
the computer stores the results of two consecutive iterations. That's it
considers that with the same size of memory it allows resolving twice
bigger problem.

I used the following algorithm [4]:

*** formula (5) ***

*** formula (6) ***

*** formula (7) ***

*** formula (8) ***

Where xi^(k) is the value of the i-th variable (in my case the value
potential at some point in the network) after the k-th iteration.
ω ∈ (0, 2) is the extrapolation parameter on which the convergence rate depends.
x and so is the value i-th variables calculated using the Gauss-Seidel iteration.

The calculation of the value by Gauss-Seidel iteration ([5], p. 44) is in my
measure (if we use the general equation for the potential at one node in the computational
network written above):

*** place a figure here ***

## Implementation

The program is written in the C programming language and comprises about 4800 lines
code. Used from the command line on UNIX-like operating systems
(Tested on Debian GNU / Linux and Darwin / Mac
OS X).

The source code is available at
http://www.tablix.org/~avian/nelma.

### Using the program

Program parameter syntax:

nelma-cap [ -s STANDOFF ]
          [ -n ITERATIONS ]
          [ -w SOR_OMEGA ]
          [ -w MAX_ERROR ]
          [ -d ]
          [ -v VERBOSITY ]
          config_file.em

<b>-s STANDOFF</b>
   Determination of the minimum distance between the earthed box and
   translator.
   STANDOFF is a positive integer.

<b>-n ITERATIONS</b>
   Determining the minimum number of iterations.
   ITERATIONS is positive integer.

<b>-w SOR_OMEGA</b>
   Value of extrapolation parameter for SOR iteration.
   SOR_OMEGA is a real number between 0 and 2.

<b>-w MAX_ERROR</b>
   Determination of the convergence criterion (maximum relative
   error in capacitance calculation).
   MAX_ERROR is a real number between 0 and 1.

<b>-v VERBOSITY</b>
   The eloquence level of the program.
   VERBOSITY is a positive whole number.
   The larger it is, the more messages the program displays on the screen.

<b>-d</b>
   A flag that includes printing the calculated electric field to a file
   on the hard disk.

<b>config_file.em</b>
   The path to the file that contains a description of the geometry of
   the print circuit.

After the calculation is completed, the program prints the capacitance
values ​​to a standard output (usually a terminal screen) in a format that
is compatible with SPICE circuit simulation programs.

Example:

<hr>
<pre>
* Created by NELMA capacitance calculator, version 2.0
* Wed Sep 27 18:08:31 2006
*
* Command line: nelma-cap -s 50 -n 100 -w 1.8 -e 0.02 -d layout.em
C0001 gnd line2 3.289585e-12
* numerical error: +/- 1.18e-13 (   4%)
C0002 gnd line1 3.688931e-12
* numerical error: +/- 1.29e-13 (   3%)
C0102 line2 line1 1.026117e-14
* numerical error: +/- 2.47e-17 (   0%)
</pre>
<hr>

Taking a line, for example

<hr>
<pre>
C0001 gnd line2 3.289585e-12
* numerical error: +/- 1.18e-13 (   4 %)
</pre>
<hr>

They mean that the capacitance between the nodes gnd and
line2 3.29 pF and that the estimated relative calculation error was 4%
(i.e. an error resulting from the placement of a grounded box and not
from the resolution of linear equations).

If the program has also been given the -d flag, the program will print
on the disk also a large number of files with calculated values ​​of
electrical potential and fields.

Names of printed files:

<b>imevozlisca-x</b>
   Calculated field in cross section x = Sx/2 when
   a node was connected to the voltage with the name of the node.

<b>imevozlisca-y</b>
   Calculated field in cross section y = Sy/2 when
   a node was connected to the voltage
   with the name of the node.

<b>imevozlisca-imeplasti</b>
   Calculated field v cross section z = with the name of the layer when
   toast connected node with name imevozlisca. z imeplasti is z coordinate
   the middle of a layer named layer name.

Line format in printed files:

<hr>
<pre>
i j field_ij potential_ij con_ij
</pre>
<hr>

<b>i</b>
   The first coordinate of the point (depending on the cross section).

<b>j</b>
   The second coordinate of the point (depending on the cross section).

<b>field_ij</b>
   The magnitude of the electric field at this point.

<b>potential_ij</b>
   The value of the electric potential at this point.

<b>con_ij</b>
   Value indicating whether this point is part of boundary conditions (1)
   but not (0).

### Description of circuit geometry

The program reads the geometry of the circuit from a text file and one
or more images.
PNG files.

The text file consists of several parts that describe the materials that
are used in printed circuit board, connection geometry and the like.

The following is a more detailed description of the parts of the file on
one of the examples that are attached to the program:

<hr>
<pre>
net line1 {
    objects = {
        "line1-front",
        "line1-back"
    }
}
net line2 {
    objects = {
        "line2-front",
        "line2-back"
    }
}
net gnd {
    objects = {
        "gnd-front",
        "gnd-back"
    }
}
</pre>
<hr>

The first part of the file describes the nets of the circuit.
The node may be derived from one or more objects.
In this case, three are defined nodes (line1, line2 and gnd), each
consisting of two objects (one object for each side of the printed
circuit board).

<hr>
<pre>
object gnd-front {
    position = { 0, 0 }
    material = "copper"
    type = "image"
    role = "net"
    file = "front.png"
    file-pos = { 30, 30 }
}
object gnd-back {
    position = { 0, 0 }
    material = "copper"
    type = "image"
    role = "net"
    file = "back.png"
    file-pos = { 30, 30 }
}

...
</pre>
<hr>

The next part of the file describes the objects.
An object is defined as a part of space that is filled with a certain
material.
In this case, they are objects of copper (definition material) and their
lower left edge is placed in coordinate starting point.

Objects are defined here as two-dimensional structures.
They get the third dimension (height) from the layer in which they are.

The geometry of the object is loaded from an image file that contains a
mask for etching each layer of the printed circuit board (in this case,
the file front.png and back.png).
The file-pos definitions contain the coordinates of the objects
on picture.

*** place a figure here ***

Figure 2: Determining the geometry of an object from a mask.

file-pos points to one point on the object, and the program then
determines the geometry using the flood-fill algorithm
object from the image.

The program also supports the geometry of objects for verification
purposes without an image file.
In this case, the type parameter is equal to the rectangle
or circle.

<hr>
<pre>
material composite {
    type = "dielectric"

    permittivity = 3.54e-11
    conductivity = 1e-10
    permeability = 0.0
}

material air {
    type = "dielectric"

    permittivity = 8.85e-12
    conductivity = 1e-10
    permeability = 0.0
}

material copper {
    type = "metal"

    permittivity = 0.0
    conductivity = 5.96e7
    permeability = 0.0
}
</pre>
<hr>

The following file defines the materials used in the printed circuit
board.
Dielectricity (As/Vm) and conductivity are given for each material
(S/m) and permeability (Vs/Am).
Currently, only the dielectric data is used.

<hr>
<pre>
layer air-top {
    height = 50
    z-order = 10

    material = "air"
}

layer component {
    height = 3
    z-order = 6

    material = "air"

    objects = {
        "line1-back",
        "line2-back",
        "gnd-back"
    }
}

layer substrate {
    height = 30
    z-order = 5

    material = "composite"
}

layer solder {
    height = 3
    z-order = 4

    material = "air"
    objects = {
        "line1-front",
        "line2-front",
        "gnd-front"
    }
}

layer air-bottom {
    height = 50
    z-order = 0

    material = "air"
}
</pre>
<hr>

The definition of layers follows.
Each layer has a defined height, height order (z-order) and base layer
material (material).
They are then in these layers previously defined objects added.

<hr>
<pre>
space test {
    step = { 4.2333e-5, 4.2333e-5, 1e-4 }

    layers = {
        "air-top",
        "component",
        "substrate",
        "solder",
        "air-bottom"
    }
}</pre>
<hr>

The last part of the file defines the step of the computational grid
(in meters) and into space adds previously defined layers.

## Results

I checked the proper operation of the program in several ways.
First I s program calculated the capacitances of simple structures for
which it is possible accurately or approximately calculate the
capacitance also by the analytical path.
I then compared the numerical results with the analytical ones.

As another method of verification, I used structures that are symmetric
after one or more axes.
In this case, I compared the numerically calculated capacitances, which
should be the same due to the symmetry.

### Comparison with analytical calculations

Figure 3 shows a graph of the capacitance of a flat plate capacitor
in the shape of a square depending on the side of the square.
The plates were in this case perpendicular to the z axis (parallel to
the layers of the printed circuit board).
For analytical the capacitance calculation used the following equation,
which does not take into account edge effects:

*** formula (9) ***

Where a is the side of the plate, d is the distance between the plates
and ε is the dielectric constant of the material between the plates.
Due to the edge effects, we expect it to be numerical calculated
capacitance slightly greater than analytically calculated.

Figure 4 again shows a graph of the capacitance of a flat capacitor,
with the difference that this time the plates lay perpendicular to the
x-axis (i.e. they intersected the layers printed circuit board).
We expect these results to be the same as the previous one
calculation.

Figure 5 shows a capacitance graph of a flat capacitor with round plates.
The plates were perpendicular to the z-axis.
The Kirchhoff equation, which also takes into account the margins, was
used for the analytical calculation of capacitance effects:

*** place a figure here ***

Figure 3: Capacitance dependence C of a planar flat capacitor v
depending on the side of the plates a.

*** place a figure here ***

Figure 4: Dependence of capacitance C of lateral flat capacitor v
depending on the side of the plates a.

*** formula (10) ***

Where r is the radius of the plates, d is the distance between the
plates and ε is the dielectric constant of the material surrounding the
slabs.
Since the equation used also takes into account edge effects, we expect
the numerical results to be the same as the analytical to numerical ones
errors exactly.

*** place a figure here ***

Figure 5: Dependence of capacitance C of a flat capacitor as a function
of the radius of the plates r.

Figure 7 shows a graph of the capacitance of the microstrip line against
mass depending on its width.
The following approximation was used for the analytical calculation of
capacitance [6]:

*** formula (11) ***

Figure 9 shows a graph of the capacitance of the buried microstrip line
against the mass depending on its width.
For analytical capacitance calculation the following approximation [6]
has been used:

*** formula (12) ***

*** place a figure here ***

Figure 6: Microstrip line.

*** place a figure here ***

Figure 7: Dependence of capacitance C on the mass of a microstrip line
as a function of its width w.

### Symmetry test

When testing the symmetry, I symmetrically placed 8 pieces of conductor
in the space in the form of a cube (Figure 10) and calculate the
capacitance between them.
Because of the symmetry all capacitances should be along the sides of
the cube (this includes, for example capacitance between net1 and net2)
the same.
They should also be with each other the same capacitances along the
diagonals of the plane of the cube (this includes, for example,
capacitance between net1 and net4) and along spatial diagonals
for example capacitance between net1 and net8).

The results of the calculation are shown in Tables 1, 2 and 3.
In the calculation, the angle was convergence criterion for SOR
iteration set relative capacitance error 1% or less.

## Conclusion

Tests have shown that the selected numerical methods work quite well
when calculating capacitance. All tests gave the expected results.

The capacitance calculations for both types of microstrip lines stand
out the most.
The reasons for these deviations could be the following:

- Analytical terms in these two cases are only rough approximations, as
  they are accurate analytical treatment of these two structures is not
  possible.

- In these two calculations, the simplification that comes most to the
  fore we do it by placing a grounded box (one conductive structure is
  much larger than the other).

- There may be a bug in the program (the likelihood of this is small as
  they are other tests give accurate results).

However, the magnitude of the error in these two cases (which are
otherwise still closest to the conditions in a real printed circuit
board) are taken as a typical size errors that we must take into account
when using this program.

*** place a figure here ***

Figure 8: Buried microstrip line

*** place a figure here ***

Figure 9: Dependence of capacitance C on the mass of the buried
microstrip line depending on its width w.

*** place a figure here ***

Figure 10: Arrangement of conductors for symmetry test.

<hr>
<pre>
C0007 net8 net1 3.331270e-15
C0106 net7 net2 3.331266e-15
C0205 net6 net3 3.331267e-15
C0304 net5 net4 3.324573e-15
</pre>
<hr>

Maximum deviation: 6.697e-18 (0.2%).

Table 1: Capacitances along the spatial diagonals of the cube in the
symmetry test.

<hr>
<pre>
C0003 net8 net5 6.062178e-15
C0005 net8 net3 6.162798e-15
C0006 net8 net2 6.162798e-15
C0102 net7 net6 6.069830e-15
C0104 net7 net4 6.162416e-15
C0107 net7 net1 6.162765e-15
C0204 net6 net4 6.162416e-15
C0207 net6 net1 6.162764e-15
C0305 net5 net3 6.153117e-15
C0306 net5 net2 6.153118e-15
C0407 net4 net1 6.069553e-15
C0506 net3 net2 6.069858e-15
</pre>
<hr>

Maximum deviation: 1.0062e-16 (1.6%).

Table 2: Capacitances along the diagonals of the cube surfaces in the
symmetry test.

<hr>
<pre>
C0001 net8 net7 1.382707e-14
C0002 net8 net6 1.382707e-14
C0004 net8 net4 1.392616e-14
C0103 net7 net5 1.381599e-14
C0105 net7 net3 1.392612e-14
C0203 net6 net5 1.381598e-14
C0206 net6 net2 1.392612e-14
C0307 net5 net1 1.391258e-14
C0405 net4 net3 1.382710e-14
C0406 net4 net2 1.382710e-14
C0507 net3 net1 1.382706e-14
C0607 net2 net1 1.382706e-14
</pre>
<hr>

Maximum deviation: 9.906e-17 (0.7%).

Table 3: Capacitances by sides of the cube in the symmetry test.

## Literature

[1] Hong, W. et al.: A Novel Dimension-Reduction Technique for the
    Capacitance Extraction of 3-D VLSI Interconnects. MTT,
    Vol. 46, No. 8, pages 1037-1044.

[2] Beeftink, F. et al.: Accurate and Efficient Layout-to-Circuit
    Extraction for High-Speed MOS and Bipolar/BiCMOS Integrated
    Circuits. ICCD Proceedings 1995, pages 360-365.

[3] Sinigoj, A. R.: ELMG polje. Ljubljana, Založba Fakultete za
    elektrotehniko, 1996.

[4] Noel, B.; Shirley M.: Successive Overrelaxation Method. 2002.
    (citirano 10. 1. 2006). Available at
    http://mathworld.wolfram.com/SuccessiveOverrelaxationMethod.html

[5] Orel, B.: Osnove numerične matematike. Ljubljana, Založba FE in FRI,
    199.

[6] IPC-D-317A: Design Guidelines for Electronic Packaging Utilizing
    High-Speed Techniques. Jan 1995, Section 5, pages 13-36.