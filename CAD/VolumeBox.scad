pr = 9/2.0;
pinr = 3.3/2.0;
dial_r = 40.1 / 2.0;

w = 88;
h = 68;
d = 2;

$fn = 50;


//box_h = 13 + 1.2;
box_h = 20 + 1.1;

neji_ox = -w/2 + 4;
neji_oy = h/2 - 4;
neji_oz = -box_h +2;


module neji() {
    difference() { 
     cylinder(5, 3, 3, true);
     translate([0, 0, -2])cylinder(5, 1, 1, true);
    }
}

module neji0() {
 //translate([0, 0, 0.25])cylinder(0.5, 1, 1.5, true);
 translate([0, 0, 0])cylinder(1, 1, 1.5, true);
}

/// omote
/*
difference() {
    union(){
        difference() {
            translate([13, 0, 0])union() {
                   cube([w, h, d], true);
                   translate([neji_ox, neji_oy, neji_oz]) neji();
                   translate([neji_ox, -neji_oy, neji_oz]) neji();
                   translate([-neji_ox, -neji_oy, neji_oz]) neji();
                   translate([-neji_ox, neji_oy, neji_oz]) neji();
                }
            union(){
                cylinder(5, pr, pr, true);
                translate([-9.6, 0, 0])cylinder(5, pinr, pinr, true);
                //translate([-9.5, 0, 0])cylinder(5, pinr, pinr, true);
                //translate([0, 9.4, 0])cylinder(5, pinr, pinr, true);
                //translate([0, -9.3, 0])cylinder(5, pinr, pinr, true);
                //translate([8, 8.0, 0])cylinder(5, pinr, pinr, true);
                //translate([0, 0, 1])cylinder(0.3, dial_r, dial_r, true);
                translate([38, 0, 0])cube([11.5, 22.5, 5], true);
                translate([38, 8, -1])cube([24, 48, 2], true);
                translate([20, 0, 1]) cube([10, 0.5, 0.5], true);
            }
        }

        translate([13, 0, -box_h / 2 - 1])difference() {
           cube([w, h,  box_h], true);
           union() {
                cube([w-4, h-4, box_h], true);
                translate([0, 0, -box_h / 2 ])cube([w-2, h-2, 1], true);
            }
        }
    }

    translate([38, 20, -3])cube([12, 50, 6], true);
}
 */
 


/// Urabuta
translate([13,0, -box_h -0.1])
difference() {
    cube([w-2-0.2, h-2 -0.2, 1], true);
    union() {
        translate([neji_ox, neji_oy, 0]) neji0();
        translate([neji_ox, -neji_oy, 0]) neji0();
        translate([-neji_ox, -neji_oy, 0]) neji0();
        translate([-neji_ox, neji_oy, 0]) neji0();
    }
}


//neji0();


