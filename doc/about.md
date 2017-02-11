![alt text]: (logo.png "Nelma logo")

# About

Nelma is a (command line) tool for numerically calculating various
electrical properties of printed circuit boards or similar objects
composed of conductors and dielectrics (however code is optimized for
circuit board-like geometry).

It is currently capable of calculating capacitances between objects -
nets on a PCB. It returns a spice-compatible description of an
equivalent circuit of stray capacitances that can be for example used
for more accurate circuit simulation. Alternatively it can also produce
field data that can be plotted for example with Gnuplot.

Support for calculating resistances existed for a while but was later
removed because it didn't receive much testing.

Nelma is available under the GNU General Public License version 2.

# What you need to run it

You will need:

    * A computer with as much RAM as you can get (1GB is minimum for any
      serious work),
    * A UNIX-like operating system,
    * C compiler and the following libraries: libpng (developed on
      1.2.8), libconfuse (developed on 2.5).

# How it works

The capacitances are calculated by:

   1. Connecting one net to a known voltage (1V) and other nets to
      ground.
   2. Calculating the electric potential throughout the object with
      finite difference method.
   3. Calculating the charge accumulated on nets by calculating electric
      flux. Electric flux is calculated through a surface that is one
      grid unit away from the net. This means that different objects in
      a net should be at least three grid units away from each other to
      get reliable results.
   4. Dividing charge by voltage.

The algorithm is therefore of O(n2) time complexity regarding the number
of nets. This way we also get to calculate each capacitance twice. The
difference between these two calculations is used as an estimation of
numerical error.

The finite difference method allocates memory that is proportional the
cube of dimensions. The field solving algorithm itself is of O(n3) time
complexity.

The system of linear equations that is produced by the finite difference
method is currently solved with a SOR iterative algorithm with a
constant extrapolation factor.

This is a result of my term project. I have a paper (in Slovene) that
explains in detail methods used by the program to calculate the
capacitances and methods I used to verify the results.
Send me a mail if you want to read it.

# Where to get it

You can get source code of stable releases from the following location:

    http://www.tablix.org/~avian/nelma/releases

Nelma is also available from NetBSD's pkgsrc collection:

    ftp://ftp.netbsd.org/pub/NetBSD/packages/pkgsrc/cad/nelma/README.html

Keep in mind that this is a work in progress. Read the README file for
installation instructions and a list of known problems.

Any Feedback is welcome.

# FAQ

**nelma-cap crashed**

Most likely there's an error in your configuration file. Run with "-v 256" option to turn up verbosity. Look for any error messages before the crash

It's also possible that you don't have enough RAM.

Do you have a material with 0.0 permittivity defined in your config file?

**How can I get my PCB layout into nelma-cap?**

There's a Nelma export HID for PCB.

You can currently get [pcb] (https://sourceforge.net/projects/pcb/) from
SourceForge.

In PCB select "File" - "Export layout" - "Nelma".

# Contact

Please send bug reports or anything else related to Nelma to the
author's email address tomaz.solc@tablix.org 

