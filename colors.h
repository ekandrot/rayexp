//
//  colors.h
//  rayexp
//
//  Created by reed on 9/30/15.
//
//

#pragma once

struct color4 {
    color4() : r(0), g(0), b(0), a(1) {}
    color4(double _r, double _g, double _b, double _a) : r(_r), g(_g), b(_b), a(_a) {}
    inline color4& operator+=(const color4& rhs) {
        r+=rhs.r; g+=rhs.g; b+=rhs.b;
        return *this;
    }
    double r, g, b, a;
};
inline color4 operator+(const color4& a, const color4& b) {
    return {a.r+b.r, a.g+b.g, a.b+b.b, 1};
}
inline color4 operator*(double a, const color4& b) {
    return color4(a*b.r, a*b.g, a*b.b, 1);
}
inline color4 operator/(const color4& b, double a) {
    return (1.0/a)*b;
}

