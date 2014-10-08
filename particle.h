//
//  particle.h
//  sph
//
//  Created by Paul Francis Cunninghame Mathews on 8/09/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#ifndef sph_particle_h
#define sph_particle_h
#include "point.h"
#include <vector>
#include <cmath>
#include <random>


class particle {
public:
    particle() : mPos(),
                 mVel(),
                 mAngVel(),
                 mSize(0),
                 mDob(0)
    {}
    
    
    point3 mPos; // position, in world coords
    point3 mVel; // velocity in word coords
    point3 mAngVel; // angular velocity
    float mSize; // how big?
    int mDob; // date of birth
    
};
#endif
