/*
    Emitter buttress.
 */

include <buttress.scad>

//buttress(crystal="tip", pcb=6, biasRight=false, lowerWiring=false);    // 1
//buttress(crystal="body", pcb=12, biasRight=true, upperWiring=true);  // 2 x2
buttress(crystal="body", pcb=12, biasRight=true, upperWiring=true, crystalHolder=9.5); // 3

