//
//  world.cpp
//  
//
//  Created by ekandrot on 9/23/15.
//
//

#include "maths.h"
#include "world.h"
#include <math.h>
#include <limits>       // std::numeric_limits


void world::add_sphere(coord3 center, double r, color4 c) {
    sphere s;
    s.center = center;
    s.r = r;
    s.color = c;
    objs.push_back(s);
}


color4 world::cast_ray(const ray3& ray __attribute__((unused))) {
    coord3 light = unit_vector({0.35,0.35,1});
    double tmin = 0;
    double tmax = std::numeric_limits<double>::max();
    double tcurrent = tmax;
    coord3 normal;
    coord3 where;
    color4 returnColor;
    bool hit(false);
    for (sphere& s : objs) {
        coord3 temp = ray.origin - s.center;
        double a = dot(ray.direction, ray.direction);
        double b = 2*dot(ray.direction, temp);
        double c = dot(temp, temp) - sqr(s.r);
        
        double discriminant = b*b-4*a*c;
        if (discriminant >= 0) {
            discriminant = sqrt(discriminant);
            double t = (-b - discriminant) / (2*a);
            if (t < tmin) {
                t = (-b + discriminant) / (2*a);
            }
            if (t >= tmin && t <= tmax && t < tcurrent) {
                tcurrent = t;
                returnColor = s.color;
                where = ray.origin + t * ray.direction;
                normal = unit_vector(where - s.center);
                hit = true;
            }
        }
    }
    if (!hit) {
        return returnColor;
    }
    double phong = dot(normal, unit_vector(light-where));
    if (phong <=0) {
        return color4();
    }
    return phong*returnColor;
}
