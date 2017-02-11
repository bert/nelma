/* Analytical calculation of capacitance between net1 and net2: 

   nelma-cap results:

   result	maxerr	omega	stoff	iterations
   ------------ ------- ------- ------- -----------
   3.143871e-11 0.01	1.7	20	20

*/

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

object p1 {
	position = { 0, 0 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 100, 100 }
}

object p2 {
	position = { 0, 0 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 100, 100 }
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

layer air-top {
	height = 20
	z-order = 10 

	material = "air"
}

layer component {
	height = 5
	z-order = 9

	material = "air"

	objects = {
			"p1"
	}
}

layer substrate {
	height = 3
	z-order = 8

	material = "air"
}

layer solder {
	height = 5
	z-order = 7

	material = "air"

	objects = {
			"p2"
	}
}

layer air-bottom {
	height = 20
	z-order = 6

	material = "air"
}

space test {
	/* 1mm x 1mm x 0.1mm */
 	step = { 1e-3, 1e-3, 1e-3 } 

	layers = { 
			"air-top", 
			"component",
			"substrate",
			"solder",
			"air-bottom"
	} 
}
