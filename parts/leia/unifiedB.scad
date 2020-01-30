use <../commonUnified.scad>
use <../shapes.scad>
use <../inset.scad>
use <testplate.scad>
include <dim.scad>

DRAW_FORE = false;
DRAW_AFT  = true;

EPS = 0.01;
EPS2 = EPS*2;

SPKR_Z = 10;
INCHES_TO_MM = 25.4;

N_BATTERY_BAFFLES = nBafflesNeeded(DZ_BUTTRESS, "18650");
N_TOTAL_BAFFLES = floor(DZ_FORE / (DZ_BUTTRESS * 2)) - 1;
N_POST_BAFFLES = N_TOTAL_BAFFLES - N_BATTERY_BAFFLES;

TRANSITION_CUTOUT = 10.0;
Z_END_INSET   = M_T - DZ_THREAD - 2.0;

DZ_T_RING = 4.0;
Z_T_RING = M_T - DZ_T_RING;
JOINT = 8;

*color("red") translate([0, 0, M_T - 4])
    tube(h=4, do=D_INNER_AFT, di=D_INNER_AFT - 4);

module drawFore()
{
    translate([0, 0, M_FORE]) {
        baffleMCBattery(D_INNER_FORE,
            N_BATTERY_BAFFLES,
            DZ_BUTTRESS,
            nPostBaffles=N_POST_BAFFLES,
            bridgeStyle=1);

        difference() {
            translate([0, 0, -DZ_T_RING]) {
                tube(h=DZ_T_RING, do=D_INNER_AFT, di=D_INNER_FORE-4, $fn=80);
            }
            translate([-TRANSITION_CUTOUT/2, -50, -DZ_T_RING - EPS]) {
                cube(size=[TRANSITION_CUTOUT, 100, DZ_T_RING + EPS2]);
            }
        }
    }
    translate([0, 0, Z_T_RING]) rotate([180, 0, 0])
        keyJoint(JOINT, D_INNER_AFT, D_INNER_AFT - 4, false);
 
    // Connect up.
    for(i=[1:2]) {
        translate([0, 0, M_FORE + zLenOfBaffles(N_TOTAL_BAFFLES, DZ_BUTTRESS) - (i * 2 + 1) * DZ_BUTTRESS]) {
            translate([-10, 3, 0])
                cube(size=[20, 7.5, DZ_BUTTRESS]);
        }
        translate([0, 0, M_FORE + zLenOfBaffles(N_TOTAL_BAFFLES, DZ_BUTTRESS) - DZ_BUTTRESS]) {
            tube(h=DZ_BUTTRESS, do=D_INNER_FORE, di=D_INNER_FORE - 6);
        }
    }
}

module drawAft()
{
    Z_START_INSET = Z_END_INSET - DZ_INSET;
    SPACE = 8.0;

    Z_START_SECTION = Z_START_INSET - SPACE;
    Z_END_SECTION   = Z_END_INSET + SPACE;

    DZ_SECTION = Z_END_SECTION - Z_START_SECTION;

    DZ_BOLT   = DZ_SECTION / 2;
    DZ_PORT   = DZ_SECTION / 2 + 12.0;
    DZ_SWITCH = DZ_SECTION / 2 - 12.0;

    DY_INSET = D_INNER_AFT/2 - yAtX(DX_INSET/2, D_INNER_AFT/2);

    *color("lightgrey") translate([0, D_INNER_AFT/2 - DY_INSET, Z_START_INSET]) {
        switchPlate();
    }

    difference() 
    {
        union() {
            // stock = 6.35 (1/4 inch)
            translate([0, 0, Z_START_SECTION]) {
                insetHolder(
                    D_INNER_AFT,
                    D_OUTER_AFT,
                    DX_INSET,
                    DZ_SECTION,
                    DZ_BUTTRESS,
                    Z_START_INSET - Z_START_SECTION, 
                    Z_END_INSET - Z_START_SECTION,     
                    DZ_BOLT,  
                    DZ_PORT,
                    DZ_SWITCH,
                    roundRect = 3.175/2,
                    frontBridge=false,
                    bridgeStyle=2,
                    dyInset=DY_INSET
                );
            }
            translate([0, 0, SPKR_Z]) difference() {
                tube(h=DZ_BUTTRESS, do=D_INNER_AFT, di=D_INNER_AFT - 6);
            }
        }
        X_MC = 0.5 * INCHES_TO_MM;
        translate([-X_MC/2, -20, 0])
            cube(size=[X_MC, 20, 50]);
        translate([0, -11, 0]) scale([1, 1, 2])
            color("green") mc();
        translate([0, 0, Z_T_RING]) rotate([180, 0, 0])
            keyJoint(JOINT, D_INNER_AFT, D_INNER_AFT - 4, true);

        dotstarZ = dotstarStripZLen(4);
        dotstarX = 12.4;    // of the strip, not the LED
        rotate([0, 0, -60])
        {
            translate([-dotstarX/2, D_INNER_AFT/2 - 2.0, DZ_BOLT + Z_START_SECTION - dotstarZ/2])
                cube(size=[dotstarX, 10, dotstarZ]);
            DX_WIRE = 8;
            translate([-DX_WIRE/2, D_INNER_AFT/2 - 5.0, Z_START_SECTION + DZ_BUTTRESS * 11])
                cube(size=[DX_WIRE, 20, DZ_BUTTRESS]);
        }
    }
    for(i=[0:2]) {
        translate([0, 0, i * 2 * DZ_BUTTRESS + SPKR_Z]) {
            oneBaffle(D_INNER_AFT,
                DZ_BUTTRESS,
                battery=false);
        }
    }
    translate([0, 0, 1]) {
        speakerHolder(D_INNER_AFT, SPKR_Z-1, 2, "std28");
    }
    //color("red") translate([0, 0, M_T - DZ_THREAD]) tube(h=DZ_THREAD, do=D_INNER_AFT-0.5, di=D_INNER_AFT-1);
}

if (DRAW_FORE) drawFore();
if (DRAW_AFT) drawAft();

//color("red") battery(D_INNER_FORE, "18650");
//translate([0, -11, SPKR_Z - 2]) color("green") mc();