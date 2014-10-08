//
//  vorton.h
//  vtpartsim
//
//  Created by Paul Francis Cunninghame Mathews on 25/09/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#ifndef vtpartsim_vorton_h
#define vtpartsim_vorton_h
#include "particle.h"
#include <algorithm>

static const float ONE_OVER_FOUR_PI = 1.0/(4* 3.14159265359);




/** Vorton, a fancy particle */
class vorton : public particle
{
    
public:
    
    // Gets the contribution of this vorton to something at otherPos
    // adds result to otherVel
    void get_velocity_contribution( vec3 &otherVel, const point3 &otherPos ) const
    {
        // this is the stuff from gourlay
/*        vec3 diff = otherPos - mPos;
        // potentiall make this faster
        float dist = diff.length();
        float oneoverdist = 1.0/dist;
        float weight = (dist < mRadius)? oneoverdist / mRadius*mRadius
                                       : oneoverdist / (dist*dist);
        vec3 cross = crossproduct(mVorticity, diff)*weight;
        
        otherVel += (ONE_OVER_FOUR_PI * (8.0f * mRadius * mRadius * mRadius)* cross);//*weight;*/
        // end gourlay intel stuff
        // begin some extra contribution so the particles follow the vortons
        //otherVel += diff.normalise() * - std::min(weight*0.01, 0.001);
        
        // or we could ghetto it
        // we want to add a contribution to otherVel based on rotating otherPos around
        // mVorticity, with size of the velocity contribution based on both the length of
        // mVorticity and inversely related to the distance between this vorton and
        // this vorton
        
        // let us get the difference in location between us and them
        vec3 diff = otherPos-mPos;
        // the distance is the length of this vector
        float dist = diff.length();
        vec3 ndiff = diff * (1.0/dist);
        // the direction of the component we are adding to the velocity
        // is going to be the cross product of the difference and the vorticity
        vec3 cross = crossproduct(mVorticity, ndiff);
        float weight = (dist > mRadius)?(1.0/(dist*0.2 + dist*dist*0.8)) * 0.02 : 1.0/mRadius;
        otherVel = otherVel + (cross*weight);
        otherVel = otherVel + (mVel - diff).normalise() * weight * .1;
    }
    
    float mRadius = 0.01; // radius of a vorton, used to avoid weirdness when too close
    vec3 mVorticity;
    
    point3 &position() {
        return particle::mPos;
    }
};

#endif
