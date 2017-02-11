/* This file demonstrates the basic syntax of Nelma configuration files. 
 *
 * Run with: 
 *
 * $ nelma-cap read_first.em > read_first.dat
 *
 */

/* Nets are collections of objects. All objects in a net are evaluated as if
   they are electrically shorted together, even if they are not connected. 
   Capacitance is calculated between nets. */

net net1 {
	objects = {
		"p1"
	}
}

net net2 {
	objects = {
		"p2"
	}
}

/* Objects are two dimensional shapes. They get the third dimension from the
 * layer they are placed on. */

/* Objects can be rectangular, circular or loaded from a PNG image file */

/* Objects can have different roles: capacitance is calculated between nets, 
 * while resistance is calculated between pins. You can also set role to "none", * which means that this object is a part of the surrounding (screw, casing, 
 * etc.) */

object p1 {
	/* Grid point coordinates of the bottom left corner of the circle */
	position = { 25, 25 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 50, 50 }
}

object p2 {
	/* Grid point coordinates of the upper left corner of the rectangle */
	position = { 25, 25 }

	material = "copper"

	type = "rectangle"
	role = "net"

	/* Size of the rectangle in grid coordinates */
	size = { 50, 50 }
}

/* Materials are defined by basic macroscopic electric properties. Anisotropic 
 * materials are not supported currently. */

material air {
	type = "dielectric"

	/* Permitivity units: As/Vm */
	permittivity = 8.85e-12

	/* Conductivity units: S/m 
	 * not used in capacitance calculation */
	conductivity = 1e-10

	/* Currently ignored */
	permeability = 0.0
}

material copper {
	type = "metal"

	permittivity = 0.0
	conductivity = 5.96e7
	permeability = 0.0
}

material composite {
	type = "dielectric"

	permittivity = 3.54e-11
	conductivity = 1e-10
	permeability = 0.0
}

/* Finite difference grid is separated into layers */

layer air-top {
	/* Height of this layer in grid coordinates */
	height = 30

	/* This number specifies the ordering of the layers along the z
	   axis. A lower z-order number means that the layer is lower (has
	   lower z coordinate */
	z-order = 10 

	/* This is the default material for this layer. If no objects are
	 * placed in a layer, it is filled with the default material */
	material = "air"
}

layer component {
	height = 5
	z-order = 9

	material = "air"

	/* We have a copper object surrounded with air on this layer*/
	objects = {
			"p1"
	}
}

layer substrate {
	height = 5
	z-order = 8

	material = "composite"
}

layer solder {
	height = 5
	z-order = 7

	material = "air"

	/* We have a copper object surrounded with air on this layer*/
	objects = {
			"p2"
	}
}

layer air-bottom {
	height = 30
	z-order = 6

	material = "air"
}

/* Calculation grid looks like this:
 * 
 * Orientation of the axes:
 * 
 *     ^ Z
 *     |  
 *     |  / Y
 *     | /
 *     |/
 *     +-----> X
 * 
 * Grid coordinates:
 * 
 * (size.y-1, 0)             (size.x-1,size.y-1)
 *              +--+--+--+--+
 *              |  |  |  |  |
 *              +--+--+--+--+
 *              |  |  |  |  |
 *              +--+--+--+--+
 *              |  |  |  |  |
 *              +--+--+--+--+
 *              |  |  |  |  |
 *              +--+--+--+--+
 *              |  |  |  |  |
 *              +--+--+--+--+
 *         (0,0)             (size.x-1,0)
 * 
 * Layer boundaries:
 *  
 *                 z-top2 +----
 *         /              | material 2
 *         |              +----
 * Layer 2 |              | material 2
 *         |              +----
 *         \              | material 2
 *       z-top1,z-bottom2 +----
 *         /              | material 1
 *         |              +----
 * Layer 1 |              | material 1
 *         |              +----
 *         \              | material 1
 *              z-botton1 +----
 */

space test {
	/* Steps of the grid in meters */
	step = { 1e-2, 1e-2, 1e-2 }

	layers = { 
			"air-top", 
			"component",
			"substrate",
			"solder",
			"air-bottom"
	}
}
