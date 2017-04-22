Retrieved from:

https://www.tablix.org/~avian/blog/archives/2010/07/new_nelma_release/

# New Nelma release
27.07.2010 12:27

This is a kind of a late announcement: A few weeks ago I released
version 3.2 of Nelma, the numerical electromagnetics package.
Changelog isn't very impressive, with the most significant change being
a patch that fixes compilation on libpng 1.4.x (contributed by
Thomas Klausner of the NetBSD's pkgsrc team).

To put some light in this almost forgotten project, here's a brief
description:

    Nelma is a command line tool for numerically calculating various
    electrical properties of printed circuit boards.

    It is currently capable of calculating capacitances between objects
    - nets on a PCB. It returns a spice-compatible description of an
    equivalent circuit of stray capacitances that can be for example
    used for more accurate circuit simulation. Alternatively it can also
    produce field data that can be plotted for example with Gnuplot.

    There's an export plugin available for gEDA PCB software that
    automatically creates a Nelma configuration from a PCB layout.

    Nelma is available under the GNU General Public License version 2.

I admit I haven't really used this since I left the Faculty, but back
then it did help me with a couple of projects. The emphasis was mostly
on the simplicity of the algorithm, so I stuck with the relatively
inefficient finite difference method (ouch, O(n3) complexity) instead of
the usual finite elements. Right now it could really use some love in
form of algorithm parallelization or conversion to finite elements
method. Any takers for a summer project?

Posted by Tomaž
