/*
 * MIT License
 *
 * Copyright (c) 2023 Martin Stokroos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// ezResponseBox enclosure for two buttons.
//
// This is a simple OpenSCAD design sketch for a 3D printable response box enclosure.
// This design is screwless. To fixate the pico, melt the (PLA)plastic pins downwards 
// with a (not too) hot iron nail.

length = 120; //dimensions in mm.
width = 60;
height = 28;
wt = 1.6; // wall thickness
poffs = 2.1; // pico spacers, offset from the inner edge (USB-side)
pindia = 1.6; // pico fixation pin diameter
knob_spacing = 34;

difference(){
    union(){
        difference(){
            translate([0,0,-height/2]) roundCornersCube(length, width, height, 4.0); //outline
            translate([0,0,-height/2-wt]) roundCornersCube(length-2*wt, width-2*wt, height, 4.0); //cutout
        }
        // spacers for the PICO
        translate([5.7, -width/2+wt+poffs, -6.0]) cylinder(d=3.5, h=5, center=false, $fn=150); //USB side
        translate([-5.7, -width/2+wt+poffs, -6.0]) cylinder(d=3.5, h=5, center=false, $fn=150); //USB side
        translate([5.7, -width/2+wt+51-poffs, -6.0]) cylinder(d=3.5, h=5, center=false, $fn=150);
        translate([-5.7, -width/2+wt+51-poffs, -6.0]) cylinder(d=3.5, h=5, center=false, $fn=150);
        
        translate([5.7, -width/2+wt+poffs, -8.0]) cylinder(d=pindia, h=6, center=false, $fn=150); //USB side
        translate([-5.7, -width/2+wt+poffs, -8.0]) cylinder(d=pindia, h=6, center=false, $fn=150); //USB side
        translate([5.7, -width/2+wt+51-poffs, -8.0]) cylinder(d=pindia, h=6, center=false, $fn=150);
        translate([-5.7, -width/2+wt+51-poffs, -8.0]) cylinder(d=pindia, h=6, center=false, $fn=150);
    }
      
    // Mark's knob
    //translate([-knob_spacing, 0, 0]) cylinder(d=30.4, h=20, center=true, $fn=150);
    //translate([knob_spacing, 0, 0]) cylinder(d=30.4, h=20, center=true, $fn=150);
    
    // Standard knobs, use your own knob cut out pattern.
    translate([34, 0, 0])cube([12.5, 15.5, 10], center=true);
    translate([-34, 0, 0])cube([12.5, 15.5, 10], center=true);
    
    // Opening for the USB connector.
    translate([0, -width/2+0.2, -8.0/2-wt]) rotate([90,0,0]) roundCornersCube(8.0, 8, 3.0, 1);
    // Hole for pico bootsel button.
    translate([3.7, -width/2+wt+poffs+10, -3]) cylinder(d=3.2, h=6, center=false, $fn=150);
    // Hole for pico LED.
    translate([6, -width/2+wt+poffs+3, -3]) cylinder(d=2, h=6, center=false, $fn=150);
}



/*
roundCornersCube is licensed under the GNU license.

Copyright (C) 2011 Sergio Vilches

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
Contact: s.vilches.e@gmail.com


    ----------------------------------------------------------- 
                 Round Corners Cube (Extruded)            
      roundCornersCube(x,y,z,r) Where:                        
         - x = Xdir width                                     
         - y = Ydir width                                     
         - z = Height of the cube                             
         - r = Rounding radious                               
                                                              
      Example: roundCornerCube(10,10,2,1);                    
     *Some times it's needed to use F6 to see good results!   
 	 ---------------------------------------------------------- 
*/

module createMeniscus(h,radius) // This module creates the shape that needs to be substracted from a cube to make its corners rounded.
difference(){        //This shape is basicly the difference between a quarter of cylinder and a cube
   translate([radius/2+0.1,radius/2+0.1,0]){
      cube([radius+0.2,radius+0.1,h+0.2],center=true);         // All that 0.x numbers are to avoid "ghost boundaries" when substracting
   }

   cylinder(h=h+0.2,r=radius,$fn = 100,center=true);
}


module roundCornersCube(x,y,z,r)  // Now we just substract the shape we have created in the four corners
difference(){
   cube([x,y,z], center=true);

translate([x/2-r,y/2-r]){  // We move to the first corner (x,y)
      rotate(0){  
         createMeniscus(z,r); // And substract the meniscus
      }
   }
   translate([-x/2+r,y/2-r]){ // To the second corner (-x,y)
      rotate(90){
         createMeniscus(z,r); // But this time we have to rotate the meniscus 90 deg
      }
   }
      translate([-x/2+r,-y/2+r]){ // ... 
      rotate(180){
         createMeniscus(z,r);
      }
   }
      translate([x/2-r,-y/2+r]){
      rotate(270){
         createMeniscus(z,r);
      }
   }
}
