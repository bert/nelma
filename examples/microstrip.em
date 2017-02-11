/* correct capacitance: 1.527 pF/in * 400 mil = 0.61 pF = 6.1e-13 F

   (from: http://www.emclab.umr.edu/pcbtlc/microstrip.html)

   result	maxerr	omega	stoff	
   ------------ ------- ------- -------
   4.890980e-13	0.02	1.8	50	

*/
net gnd {
	objects = {
		"gnd",
	}
}

net line {
	objects = {
		"line"
	}
}

object gnd {
	position = { 0, 0 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 200, 200 }
}

object line {
	position = { 90, 0 }

	material = "copper"

	type = "rectangle"
	role = "net"

	size = { 20, 200 }
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

material composite {
	type = "dielectric"

	permittivity = 3.54e-11
	conductivity = 1e-10
	permeability = 0.0
}

layer air-top {
	height = 100
	z-order = 70 

	material = "air"
}

layer component {
	height = 5
	z-order = 60

	material = "air"

	objects = {
			"line"
	}
}

layer substrate {
	height = 70
	z-order = 30

	material = "composite"
}

layer solder {
	height = 5
	z-order = 20

	material = "air"

	objects = {
			"gnd"
	}
}

layer air-bottom {
	height = 100
	z-order = 10

	material = "air"
}

space test {
	/* 1, 2, 0.5 mil */
 	step = { 2.54e-5, 5.08e-5, 1.27e-5 } 

	layers = { 
			"air-top", 
			"component",
			"substrate",
			"solder",
			"air-bottom"
	} 
}
