//
// Created by Jacopo Gasparetto on 16/06/21.
//
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/procedural-patterns-noise-part-1/creating-simple-1D-noise
#ifndef FORTRESS_VALUENOISE1D_H
#define FORTRESS_VALUENOISE1D_H

#include <cstdlib>
#include <Constants.h>

template<typename T = float>
inline T lerp(const T &lo, const T &hi, const T &t) { return lo * (1 - t) + hi * t; }

class ValueNoise1D {
public:
    static const uint16_t kMaxVertices = fortress::consts::WINDOW_SIZE_IN_POINT * 1.5;
    static const uint16_t kMaxVerticesMask = kMaxVertices - 1;
    double r[ kMaxVertices ]{};

    explicit ValueNoise1D(unsigned seed = 2021) {
        srand48(seed);

        for (double & i : r) {
            i = drand48();
        }
    }

    double eval(const double &x) {
        // Floor
        int xi = (int)x - (x < 0 && x != (int)x);
        double t = x - xi;
        // Modulo using &
        int xMin = xi & kMaxVerticesMask;
        int xMax = (xMin + 1) & kMaxVerticesMask;

        return lerp(r[xMin], r[xMax], t);
    }
};


#endif //FORTRESS_VALUENOISE1D_H
