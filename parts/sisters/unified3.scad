use <../shapes.scad>
include <dim.scad>
use <../commonUnified.scad>

$fn = 90;
EPS = 0.01;
EPS2 = EPS * 2;

DRAW_AFT = true;
DRAW_FRONT = true;

JOINT_ANGLE = -20;
T = 4;
JUNCTION = 6;
EXTRA_BAFFLE = 2;
Z_AFT_RING = 5.5;       // also speaker holder
Z_SPACER = 2.5;
N_BAFFLES = nBafflesNeeded(H_BUTTRESS);
M_BAFFLE_FRONT = zLenOfBaffles(N_BAFFLES, H_BUTTRESS) 
    + M_POMMEL_FRONT + EXTRA_BAFFLE + Z_AFT_RING + Z_SPACER;

if (DRAW_AFT) {
    difference() {
        union() {
            translate([0, 0, M_POMMEL_FRONT + Z_AFT_RING]) {
                oneBaffle(D_AFT, Z_SPACER, false, true, false, true, cutoutHigh=false);
                translate([0, 0, Z_SPACER])
                    baffleMCBattery(D_AFT, N_BAFFLES, H_BUTTRESS, 
                                    D_AFT, H_BUTTRESS, EXTRA_BAFFLE, mcWide=19.5);
            }

            translate([0, 0, M_POMMEL_FRONT]) {
                speakerHolder(D_AFT_RING, Z_AFT_RING, 1, "cls28");
            }

            translate([0, 0, M_BAFFLE_FRONT]) {
                keyJoint(JUNCTION, D_AFT, D_AFT - T, 0.5, JOINT_ANGLE);
            }
        }
        translate([0, 0, M_POMMEL_FRONT + 1])
            speakerCLS28(true);
    }
    *translate([0, 0, M_POMMEL_FRONT + Z_AFT_RING + 3]) 
        battery(D_AFT);
    *translate([0, 0, M_POMMEL_FRONT + 1])
        speakerCLS28(true);
}

if (DRAW_FRONT) {
    // Transition part; in the aft section
    translate([0, 0, M_BAFFLE_FRONT]) {
        difference() 
        {
            W = 14;
            DZ = M_TRANSITION - M_BAFFLE_FRONT;
            //tube(h=DZ, do=D_AFT, di=D_AFT - T);

            PCB_DX = 26.94;
            CUT_DX = 22.0;
            PCB_DZ = 19.32;  
            CUT_DZ = 12;
            TO_PCB = M_DISPLAY_CENTER - M_BAFFLE_FRONT - CUT_DZ / 2;
            DZ_FIX = (CUT_DZ - PCB_DZ) / 2;

            pcbHolder(  D_AFT, T, DZ,
                        TO_PCB,
                        4,
                        [CUT_DX, 50, CUT_DZ],
                        [
                            [PCB_DX/2 -  2.04, DZ_FIX + 16.01, "buttress"],
                            [PCB_DX/2 - 24.90, DZ_FIX + 16.01, "buttress"],
                            [PCB_DX/2 -  2.04, DZ_FIX +  3.31, "buttress"],
                            [PCB_DX/2 - 24.90, DZ_FIX +  3.31, "buttress"],
                        ],
                        holeAccess = false
            );

            CUT_DX_V2 = 28;
            translate([-CUT_DX_V2/2, 4, 0]) cube(size=[CUT_DX_V2, 50, 20]);
            CUT_DX_V3 = 50;
            translate([-CUT_DX_V3/2, 4 + 1.5, 0]) cube(size=[CUT_DX_V3, 50, 20]);
            
            // Doesn't fit: oledHolder(D_AFT, T, DZ, 1, 7);
            keyJoint(JUNCTION, D_AFT, D_AFT - T, 0, JOINT_ANGLE);

            for(i=[0:2]) {
                translate([0, 0, M_DISPLAY_CENTER - M_BAFFLE_FRONT + 6 - i*6]) capsule(-160, 160);
            }
        }
    }

    OVERLAP = M_TRANSITION - M_BAFFLE_FRONT - 20.0;

    translate([0, 0, M_TRANSITION])
        forwardAdvanced(D_FORWARD,                          // diameter
                        M_LED_HOLDER_FRONT - M_TRANSITION,  // dz
                        OVERLAP,                            // overlap
                        D_AFT,                              // diameter to match up to
                        M_PORT_CENTER - M_TRANSITION,       // port
                        M_SWITCH_CENTER - M_TRANSITION      // switch
                );
}

