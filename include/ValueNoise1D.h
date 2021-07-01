//
// Created by Jacopo Gasparetto on 16/06/21.
//
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/procedural-patterns-noise-part-1/creating-simple-1D-noise
#ifndef FORTRESS_VALUENOISE1D_H
#define FORTRESS_VALUENOISE1D_H

#include <stdlib.h>

template<typename T = float>
inline T lerp(const T &lo, const T &hi, const T &t) { return lo * (1 - t) + hi * t; }

class ValueNoise1D {
public:
    static const unsigned kMaxVertices = 256;
    static const unsigned kMaxVerticesMask = kMaxVertices - 1;
    float r[ kMaxVertices ];

    ValueNoise1D(unsigned seed = 2021) {
        srand48(seed);

        for (unsigned i = 0; i < kMaxVertices; ++i) {
            r[i] = drand48();
        }
    }

    float eval(const float &x) {
        // Floor
        int xi = (int)x - (x < 0 && x != (int)x);
        float t = x - xi;
        // Modulo using &
        int xMin = xi & kMaxVerticesMask;
        int xMax = (xMin + 1) & kMaxVerticesMask;

        return lerp(r[xMin], r[xMax], t);
    }
};


#endif //FORTRESS_VALUENOISE1D_H
