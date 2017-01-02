// FIXME: pad speaker holder
// FIXME: way to insert speaker

use <../threads.scad>
use <../shapes.scad>
use <../vents.scad>
include <dim.scad>
use <buttress.scad>

M_DOTSTAR_EDGE = M_DOTSTAR - X_DOTSTAR / 2;

INCHES_TO_MM = 25.4;
MM_TO_INCHES = 1 / 25.4;
$fn = 90;
EPS = 0.01;
EPS2 = EPS * 2;

H_HEAT_SINK_THREAD = 10.0;
D_HEAT_SINK_THREAD = 20.2;  // 20.4 is loose (PHA), 20.1 tight (PLA)
R_DOTSTAR          = 90;
SPKR_OFFSET         = 7;    // distance from back of pommel to speaker

DOTSTAR_WALL       = Y_DOTSTAR + 0.5;

module innerTube()
{
    translate([0, 0, M_WAY_BACK]) {
        cylinder(d=D_AFT, h=M_TRANSITION - M_WAY_BACK);
    }
    translate([0, 0, M_TRANSITION]) {
        cylinder(d=D_FORWARD, h = H_FAR);
    }
}

module heatSink()
{
    translate([0, 0, M_LED_HOLDER_FRONT - H_HEAT_SINK_THREAD]) {
        cylinder(h=H_HEAT_SINK_THREAD + 0.2, d=D_HEAT_SINK_THREAD);
    }
}

// LED / button positive (above axis)
// thread negative (below axis)
//
module switch()
{
    D_OUTER_TOP = 13.8;
    D_INNER_TOP = 11.0;
    H_TOP       =  1.5;
    H_BODY      = 13;   // approx. connections below.

    color("yellow") {
        translate([0, 0, M_SWITCH_CENTER]) {
            rotate([R_DOTSTAR, 0, 0]) {
                translate([0, 0, Y_SWITCH]) {
                    cylinder(h = H_TOP, r1 = D_OUTER_TOP / 2, r2 = D_INNER_TOP / 2);
                    translate([0, 0, -H_BODY]) {
                        cylinder(h = H_BODY, d=D_SWITCH);
                    }            
                }
            }

        }
    }
}

module dotstars(y, strip)
{
    translate([-X_DOTSTAR/2, -y, 0]) {
        for(i=[0:3]) {
            PAD = 0.4;
            translate([-PAD/2, 0, DOTSTAR_SPACE * i - PAD/2]) {
                cube(size=[X_DOTSTAR + PAD, y, X_DOTSTAR + PAD]);
            }
        }
    }   
    if (strip) {
        X_PAD = 7;  // min 7
        END_PAD = 2.5;
        X_STRIP = X_DOTSTAR + X_PAD;
        Z_STRIP = DOTSTAR_SPACE * 3 + X_DOTSTAR + END_PAD;
        T = 3;

        translate([-X_DOTSTAR/2 - X_PAD/2, 0, 0]) {
            cube(size=[X_STRIP, T, Z_STRIP]);
        }
    }
}

module dotstarsPositioned(y, strip)
{
    rotate([0, 0, R_DOTSTAR]) {
        translate([0, -R_FORWARD + DOTSTAR_WALL + EPS, M_DOTSTAR_EDGE]) {
            dotstars(y, strip);
        }
    }    
}

module ledHolder()
{
    H = M_LED_HOLDER_FRONT - M_LED_HOLDER_BACK;
    difference() {
        translate([0, 0, M_LED_HOLDER_BACK]) cylinder(h=H, d=D_FORWARD);
        heatSink();
        dotstarsPositioned(20, true);
    }
}

module speaker(pad=0, dpad=0)
{
    translate([0, 0, -pad/2]) cylinder(h=H_SPKR_PLASTIC + pad, d=D_SPKR_PLASTIC + dpad);
    translate([0, 0, H_SPKR_PLASTIC]) cylinder(h=H_SPKR_METAL, d=D_SPKR_METAL + dpad);
}

module speakerRailJoints()
{
    // End rail joints
    R = [RAIL_ANGLE_0, RAIL_ANGLE_1];
    for(r=R) {
        rotate([0, 0, r]) {
            difference() {
                *translate([-W_RAIL/2 - RAIL_PAD/2, R_AFT - RAIL_OUTER_NOTCH, M_RAIL_START]) {
                   cube(size=[W_RAIL + RAIL_PAD, 10 - 0.1, 10]);
                }
                translate([-W_RAIL/2 - RAIL_PAD/2, R_AFT - RAIL_OUTER_NOTCH, M_WAY_BACK]) {
                   cube(size=[W_RAIL + RAIL_PAD, 10 - 0.1, H_FAR]);
                }
            }
        }
    }

}

module speakerHolder()
{
    D_SPKR_INNER = D_SPKR_PLASTIC - 4;

    // Locking ring.
    difference() {
        translate([0, 0, M_POMMEL_FRONT]) {
            difference() {
                cylinder(h=M_SPKR_RING - M_POMMEL_FRONT, d=D_AFT_RING);
                cylinder(h=M_SPKR_RING - M_POMMEL_FRONT, d=D_SPKR_INNER);
            }
        }

        // Amplifier mounting holes.
        W = 11;
        D = 2 - 0.1;
        Y = 12.2;
        translate([ W/2, Y, M_POMMEL_FRONT]) cylinder(h=4, d=D);
        translate([-W/2, Y, M_POMMEL_FRONT]) cylinder(h=4, d=D);
        
        speakerRailJoints();
    }

    // Actual holder.
    H_POMMEL = M_POMMEL_FRONT - M_POMMEL_BACK;
    translate([0, 0, M_POMMEL_BACK]) {
        difference() {
            tube(H_POMMEL, D_SPKR_INNER/2, D_POMMEL/2);
            translate([-20, 3, 0]) cube(size=[40, 40, 40]);
            translate([0, 0, SPKR_OFFSET]) {
                speaker(0.8, 0.6);
            }
            translate([0, 0, 2]) rotate([0, 0, 224]) vent2(6, 20, 8, 20, 3);
            // HACK remove translation
            translate([0, 0, -M_POMMEL_BACK]) speakerRailJoints();
        }
    }    
}

module speakerRing()
{
    PAD = 1;    // space for foam, etc.

    translate([0, 0, M_POMMEL_BACK]) {
        difference() {
            cylinder(h=SPKR_OFFSET - PAD, d=D_POMMEL);
            cylinder(h=SPKR_OFFSET - PAD, d=D_SPKR_PLASTIC - 2);
        }
    }
}

module switchAndPortHolder()
{
    POWER_X         = 11;
    POWER_Y         = 14.5;
    POWER_Z         = 10;
    POWER_OFFSET_Y  = 0.2;
    POWER_OFFSET_X  = -1;

    H = (M_LED_HOLDER_BACK - M_TRANSITION + T_TRANSITION_RING);
    T = 3;
    T_ACCEL = 2;

    intersection() {
        innerTube();

        difference() {
            union() {
                translate([-20, Y_SWITCH - T, M_TRANSITION - T_TRANSITION_RING]) {
                    cube(size=[40, T+1.5, H]);
                }
                
                // Build up port holder for standard power port.
                // (Never get to use that kind.)
                W = 2 * (M_PORT_CENTER - M_TRANSITION); // fancy; snaps into ring.
                difference() 
                {
                    translate([-W/2, Y_SWITCH, M_PORT_CENTER - W/2]) {
                        cube(size=[W, 6 - T, W]);
                    }
                    // Slope the front.
                    translate([-W/2, Y_SWITCH, M_PORT_CENTER + W/2]) {
                        rotate([-45, 0, 0]) {
                            cube(size=[W, W, W]);
                        }
                    }
                }

                // Slight build up for accelerometer mount.
                ACCEL_BACK = M_TRANSITION - T_TRANSITION_RING;
                ACCEL_FRONT = M_PORT_CENTER - 6;
                translate([-W/2, Y_SWITCH - T - T_ACCEL, ACCEL_BACK]) {
                    difference() 
                    {
                        cube(size=[W, T_ACCEL, ACCEL_FRONT - ACCEL_BACK]);
                        translate([0, 0, ACCEL_FRONT - ACCEL_BACK - T_ACCEL]) rotate([45, 0, 0]) cube(size=[W, W, W]);
                    }
                }
                
            }
            // switch
            translate([0, 0, M_SWITCH_CENTER]) {
                rotate([-90, 0, 0]) {
                    cylinder(h=20, d=D_SWITCH);
                    translate([0, 0, Y_SWITCH]) cylinder(h=20, d=D_SWITCH_TOP);
                }        
            }

            // Port (std)
            translate([0, 0, M_PORT_CENTER]) {
                rotate([-90, 0, 0]) {
                    cylinder(h=20, d=D_SMALL_PORT);
                }
            }

            // Screw holes for accelerometer
            translate([0, 0, M_TRANSITION + 4.5]) {
                W = 15;
                D = 2.5 - 0.2;
                translate([ W/2, 0, 0]) rotate([-90, 0, 0]) cylinder(h=20, d=D);
                translate([-W/2, 0, 0]) rotate([-90, 0, 0]) cylinder(h=20, d=D);
            }

            transitionRing();
            heatSink();
        }
    }
}


module dotstarHolder() {
    intersection()
    {
        innerTube();

        union()
        {
            difference()
            {
                OFFSET = 34;
                rotate([0, 0, R_DOTSTAR]) translate([-10, -R_FORWARD - 1, M_TRANSITION - T_TRANSITION_RING + OFFSET]) {
                    cube(size=[20, DOTSTAR_WALL + 2, M_LED_HOLDER_BACK - M_TRANSITION + T_TRANSITION_RING - OFFSET]);
                }
                color("yellow") dotstarsPositioned(20, true);
                /*
                translate([0, -R_FORWARD + DOTSTAR_WALL + EPS, M_DOTSTAR_EDGE]) {
                    dotstars(10);
               }*/
            }
        }
    } 
}

module forwardRail()
{
    intersection()
    {
        translate([0, 0, M_TRANSITION - T_TRANSITION_RING]) {
            cylinder(d=D_FORWARD, h=M_LED_HOLDER_BACK - M_TRANSITION + T_TRANSITION_RING);
        }
        translate([-4, -R_FORWARD, M_TRANSITION - T_TRANSITION_RING]) {
            cube(size=[8, 2.5, 100]);
        }
    }
}

module transitionRing()
{
    R = [RAIL_ANGLE_0, RAIL_ANGLE_1];
    R_INNER = D_FORWARD/2;

    difference() {
        intersection() {
            cylinder(h=H_FAR, d=D_AFT);
            union() {
                translate([0, 0, M_TRANSITION - T_TRANSITION_RING]) {
                    tube(T_TRANSITION_RING, R_INNER, D_AFT/2);
                }
                difference() {
                    translate([-20, -R_FORWARD, M_TRANSITION - T_TRANSITION_RING]) {
                        cube(size=[40, 4, T_TRANSITION_RING]);
                    }            
                    translate([-4, -R_FORWARD, M_TRANSITION - T_TRANSITION_RING]) {
                        cube(size=[8, 2.5, T_TRANSITION_RING]);
                    }
                }
                translate([-20, Y_SWITCH - 3, M_TRANSITION - T_TRANSITION_RING]) {
                    cube(size=[40, 2, T_TRANSITION_RING]);
                }   

                W_SUPPORT = 8;
                H_SUPPORT = 7;

                difference() {
                    union() {
                        for(r=[0,1]) {
                            rotate([0, 0, 180*r]) translate([R_INNER, -W_SUPPORT/2, M_TRANSITION - T_TRANSITION_RING - H_SUPPORT]) {
                                cube(size=[20, W_SUPPORT, H_SUPPORT]);
                            }
                        }
                    }                
                    rail(RAIL_ANGLE_0, 20, false);
                    rail(RAIL_ANGLE_1, 20, false);
                }  
            }
        }    
    }
}

module rail(r, extend=0, notch=true)
{
    H = M_TRANSITION - M_RAIL_START - T_TRANSITION_RING;
    M = [M_BUTTRESS_0, M_BUTTRESS_1, M_BUTTRESS_2];

    rotate([0, 0, r]) {
        difference() {
            translate([-W_RAIL/2, R_AFT - RAIL_OUTER_NOTCH, M_RAIL_START]) {
               cube(size=[W_RAIL, RAIL_OUTER_NOTCH - 0.1 + extend, H]);
            }
            for(m=M) {
                translate([-W_RAIL/2 - EPS, R_AFT - RAIL_OUTER_NOTCH, m - RAIL_PAD/2]) {
                   cube(size=[W_RAIL + EPS2, RAIL_INNER_NOTCH, H_BUTTRESS + RAIL_PAD]);
                }
            }
            // Notch front so I can orient it:
            if (notch) {
                translate([-0.5, 0, M_RAIL_START + H - 2]) cube(size=[1, 20, 2.1]);
            }
        }
    }
}

module SDcard() {
    // oriented 
}

*union() {
    ledHolder();
    switchAndPortHolder();
    dotstarHolder();
    forwardRail();
}
transitionRing();
rail(RAIL_ANGLE_0);
rail(RAIL_ANGLE_1);
translate([0, 0, M_BUTTRESS_2]) buttress();
translate([0, 0, M_BUTTRESS_1]) buttress();
translate([0, 0, M_BUTTRESS_0]) {
    difference() {
        buttress();
        translate([11.5, -9.2, 0]) cube(size=[1 + 0.2, 18 + 0.5, 2.5]);   // 1.5 partial
    }
}
speakerHolder();
*switch();
*color("yellow") heatSink();
*color("yellow") speaker();
*color("yellow") speakerHolder();
