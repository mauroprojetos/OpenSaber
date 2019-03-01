FACES   = 180;
EPSILON = 0.1;
PIN	    = 2.54;


module tube(h=1, inner=0, outer=0, di=0, do=0) {
    rInner = inner > 0 ? inner : di / 2;
    rOuter = outer > 0 ? outer : do / 2;

	if (rInner >= rOuter) echo("ERROR in tube. rOuter must be > rInner.");

	difference() {
		cylinder(h=h, r=rOuter);
		translate([0,0,-EPSILON]) {
			cylinder(h=h + EPSILON*2, r=rInner);
		}
	}
}

module roundedRect(h, d)
{
	D0 = d * 0.8;
	D1 = d * 0.2;
	H = h;

	hull()
	{
		translate([-D0/2, -D0/2, 0]) {
			cube([D0, D0, H]);
		}
		translate([-D0/2, -D0/2, 0])  cylinder(d=D1, h=H);
		translate([ D0/2, -D0/2, 0])  cylinder(d=D1, h=H);
		translate([-D0/2,  D0/2, 0])  cylinder(d=D1, h=H);
		translate([ D0/2,  D0/2, 0])  cylinder(d=D1, h=H);
	}
}

module cubePair(x, size)
{
	translate([x, 0, 0])
		cube(size=size);
	mirror([1,0,0]) translate([x, 0, 0])
		cube(size=size);
}

module polygonXY(h, points)
{
    linear_extrude(height = h) {
        polygon(points=points);
    }  
}

module polygonYZ(h, points)
{
    multmatrix(m = [ [0, 0, 1, 0],
    				 [1, 0, 0, 0],
    				 [0, 1, 0, 0],
    				 [0, 0, 0, 1]
       			   ])
    linear_extrude(height = h) {
        polygon(points=points);
    }  	
}

module polygonXZ(h, points)
{
    multmatrix(m = [ [1, 0, 0, 0],
    				 [0, 0, 1, 0],
    				 [0, 1, 0, 0],
    				 [0, 0, 0, 1]
       			   ])
    linear_extrude(height = h) {
        polygon(points=points);
    }  	
}


module zCapsule(dz0, dz1, r)
{
    hull() {
        translate([0, 0, dz0]) rotate([-90, 0, 0]) cylinder(h=50, r=r);
        translate([0, 0, dz1]) rotate([-90, 0, 0]) cylinder(h=50, r=r);
    }
}
