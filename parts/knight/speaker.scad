use <../threads.scad>
use <../shapes.scad>
include <dim.scad>

/*
	3 sections:
	TUBE_COUPLE
	SPKR_METAL
	SPKR_PLASTIC
*/

TUBE_COUPLE_H  = 3.5;
TUBE_COUPLE_D  = 25.4;
SPKR_METAL_H   = 8;
SPKR_METAL_D   = 23.6;	
SPKR_PLASTIC_H = 2.7;
SPKR_PLASTIC_D = 28;
TRAN     	   = (TUBE_COUPLE_D - SPKR_METAL_D)/2;

//intersection()
{
	difference() 
	{
		union() {
			tube(TUBE_COUPLE_H + TRAN, D_AFT/2, TUBE_COUPLE_D/2);
			translate([0, 0, TUBE_COUPLE_H]) {
				shelf(TUBE_COUPLE_D	/2, TRAN, SPKR_METAL_D/2);
				translate([0, 0, TRAN]) {
					tube(SPKR_METAL_H - TRAN, D_AFT/2, SPKR_METAL_D/2);
					translate([0, 0, SPKR_METAL_H - TRAN]) {
						tube(SPKR_PLASTIC_H, D_AFT/2, SPKR_PLASTIC_D/2);
					}
				}
			}
		}
		for(r=[0:1]) {
            S = 8;
            S2 = 4;
			rotate([0,0,r*180]) {
				translate([D_AFT/2 - S, -S/2, TUBE_COUPLE_H + 2]) {
					cube(size=[S+1, S, 20], center=false);
				}
                translate([TUBE_COUPLE_D/2 - S2, -S/2, -0.1]) {
                    cube(size=[4, S, 20]);
                }
			}
		}
	}
//	cube(size=[100,100,100]);
}