/* Analytical calculation of capacitance between net1 and net2: 

   nelma-cap results:

   result	maxerr	omega	stoff	iterations
   ------------ ------- ------- ------- -----------
   3.143794e-11 0.01	1.7	20	30

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

	size = { 5, 100 }
}

object p2 {
	position = { 8, 0 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 5, 100 }
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

layer substrate {
	height = 100
	z-order = 8

	material = "air"

	objects = { "p1", "p2" }
}

layer air-bottom {
	height = 20
	z-order = 6

	material = "air"
}

space test {
 	step = { 1e-3, 1e-3, 1e-3 } 

	layers = { 
			"air-top", 
			"substrate",
			"air-bottom"
	} 
}
