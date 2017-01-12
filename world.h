//
//  world.h
//  
//
//  Created by reed on 9/23/15.
//
//

#pragma once

#include "coords.h"
#include "colors.h"
#include <stdio.h>
#include <math.h>
#include <vector>


struct ray3 {
    coord3 origin;
    coord3 direction;
};


struct sphere {
    coord3 center;
    double r;
    color4 color;
};


struct world {
    void add_sphere(coord3 center, double r, color4 c);
    color4 cast_ray(const ray3& ray __attribute__((unused)));
    
private:
    std::vector<sphere> objs;
};
