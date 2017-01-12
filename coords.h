//
//  coords.h
//  rayexp
//
//  Created by reed on 9/30/15.
//
//

#pragma once
#include <math.h>

struct coord3 {
    coord3() : x(0), y(0), z(0) {}
    coord3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    inline coord3& operator+=(const coord3& b) {
        x+=b.x; y+=b.y; z+=b.z;
        return *this;
    }
    double x, y, z;
};
inline coord3 operator-(const coord3& a, const coord3& b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z};
}
inline coord3 operator+(const coord3& a, const coord3& b) {
    return {a.x+b.x, a.y+b.y, a.z+b.z};
}
inline coord3 operator*(double a, const coord3& b) {
    return {a*b.x, a*b.y, a*b.z};
}
inline coord3 operator/(const coord3& b, double a) {
    return (1.0/a)*b;
}
inline double dot(const coord3& a, const coord3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
inline coord3 unit_vector(const coord3& a) {
    double norm = sqrt(dot(a, a));
    return a/norm;
}
