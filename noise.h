//
//  noise.h
//  noisepart
//
//  Created by Paul Francis Cunninghame Mathews on 7/10/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#ifndef noisepart_noise_h
#define noisepart_noise_h
#include "point.h"
#include "interpolation.h"
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <cmath>

// perlin noise
class perlin {
public:
    perlin(unsigned seed = 0xface) {
        perm.resize(256);
        std::iota(perm.begin(), perm.end(), 0); // fill with ramp
        std::default_random_engine random(seed); // going to need some randoms
        std::shuffle(perm.begin(), perm.end(), random); // shuffle them
        perm.insert(perm.end(), perm.begin(), perm.end()); // and double it
    };
    
    // three dimensional noise
    float operator()(const point3 &x) {
        return (*this)(x[0], x[1], x[2]);
    };
    float operator()(float a, float b=1.0, float c=1.0) {
        int x = (int) floor(a) & 255;
        int y = (int) floor(b) & 255;
        int z = (int) floor(c) & 255; // find unit cube
        
        a = a-floor(a);
        b = b-floor(b);
        c = c-floor(c); // get some interpolation constants
        
        float u = fade(a);
        float v = fade(b);
        float w = fade(c);
        
        // wiggled cube corners
        int A = perm[x] + y;
        int AA = perm[A] + z;
        int AB = perm[A+1] + z;
        int B = perm[x+1] + y;
        int BA = perm[B] + z;
        int BB = perm[B+1] + z;
        
        // interpolate gradients
        float answer = lerp(lerp(lerp(grad(perm[AA], a,b,c),
                                      grad(perm[BA], a-1,b,c),
                                      u),
                                 lerp(grad(perm[AB], a,b-1,c),
                                      grad(perm[BB], a-1,b-1,c),
                                      u),
                                 v),
                            lerp(lerp(grad(perm[AA+1], a, b, c-1),
                                      grad(perm[BA+1],a-1,b,c-1),
                                      u),
                                 lerp(grad(perm[AB+1], a, b-1, c-1),
                                      grad(perm[BB+1], a-1, b-1, c-1),
                                      u),
                                 v),
                            w);
        
        return (answer+1.0)/2.0; // return scaled result
    };
    
private:
    std::vector<unsigned> perm;
    
    float fade(float a) { // this is important because of reasons
        return a * a * a * (a * (a * 6 - 15) + 10);
    };
    
    // some tricksy stuff here, thanks Ken
    float grad(int hash, double x, double y, double z) {
        int h = hash & 15; // lower 4 bits
        float u = (h < 8)? x : y, // if statemets are for mortals?
              v = (h < 4)? y : h == 12 || h == 14? x : z;
        return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
    }
};

/* does some of the trickiest biz, lets us query for velocity at a given position
   Designed to be subclasses in order to provide potentials with particular 
   characteristics for a given flow
 */
class curlnoise {
protected:
    float time; // simulation time
    float dx; // for the finite difference
    float dv; // for the other finite difference
    
public:
    curlnoise() : time(0), dx(1e-3), dv(5e-2) {}; // TODO: is this a sensible dx?
    virtual ~curlnoise() {}
    
    // no prizes for guessing what this does
    void advance_time(float step) {
        time += step;
    }
    
    // returns a sample from the potential field, to evaluate the derivatives of
    virtual vec3 potential(float x, float y, float z) =0;
    
//    void get_velocity(const Vec3f &x, Vec3f &v) const
//    {
//        v[0]=( (potential(x[0], x[1]+delta_x, x[2])[2] - potential(x[0], x[1]-delta_x, x[2])[2])
//              -(potential(x[0], x[1], x[2]+delta_x)[1] - potential(x[0], x[1], x[2]-delta_x)[1]) ) / (2*delta_x);
//        v[1]=( (potential(x[0], x[1], x[2]+delta_x)[0] - potential(x[0], x[1], x[2]-delta_x)[0])
//              -(potential(x[0]+delta_x, x[1], x[2])[2] - potential(x[0]-delta_x, x[1], x[2])[2]) ) / (2*delta_x);
//        v[2]=( (potential(x[0]+delta_x, x[1], x[2])[1] - potential(x[0]-delta_x, x[1], x[2])[1])
//              -(potential(x[0], x[1]+delta_x, x[2])[0] - potential(x[0], x[1]-delta_x, x[2])[0]) ) / (2*delta_x);
//    }
    
    // gets a velocity value for a position, calculated by the curl of a noise field as in Bridson 2007
    void get_velocity(const vec3 &pos, vec3 &result) {
        // we need to sample the potential field a few times (6)
        vec3 pDiffX = potential(pos[0]+dx, pos[1], pos[2]) - potential(pos[0]-dx, pos[1], pos[2]);
        vec3 pDiffY = potential(pos[0], pos[1]+dx, pos[2]) - potential(pos[0], pos[1]-dx, pos[2]);
        vec3 pDiffZ = potential(pos[0], pos[1], pos[2]+dx) - potential(pos[0], pos[1], pos[2]-dx);
        
        // finite differences yay
        result[0] = (pDiffY[2] - pDiffZ[1]) / (2*dx);
        result[1] = (pDiffZ[0] - pDiffX[2]) / (2*dx);
        result[2] = (pDiffX[1] - pDiffY[0]) / (2*dx);
        float factor = (pos[1] + 1.0f)/4.0f;
        result = result * factor;
        result[1] += 1;
    }
    
    // updates the velocity and the vorticity at the same time
    void vorticity_velocity(const vec3 &pos, vec3 &vel, vec3 &vort) {
        // the velocity is the curl of the (implicit) potential field
        // and the vorticity is the curl of the (derived) velocity field
        // so we have to sample the velocity SIX times (optimising that noise is looking like a good move)
        vec3 vPlusX,vNegX,vPlusY, vNegY, vPlusZ, vNegZ;
        vec3 p1,p2,p3,p4,p5,p6;
        p1 = p2 = p3 = p4 = p5 = p6 = pos;
        p1[0] += dv;
        p2[0] -= dv;
        p3[1] += dv;
        p4[1] -= dv;
        p5[2] += dv;
        p6[2] -= dv;
        get_velocity(p1, vPlusX);
        get_velocity(p2, vNegX);
        get_velocity(p3, vPlusY);
        get_velocity(p4, vNegY);
        get_velocity(p5, vPlusZ);
        get_velocity(p6, vNegZ);
        
        // now results
        get_velocity(pos, vel);
        vec3 vDiffX = vPlusX - vNegX;
        vec3 vDiffY = vPlusY - vNegY;
        vec3 vDiffZ = vPlusZ - vNegZ;
        vort[0] = (vDiffY[2] - vDiffZ[1]) / (2*dv);
        vort[1] = (vDiffZ[0] - vDiffX[2]) / (2*dv);
        vort[2] = (vDiffX[1] - vDiffY[0]) / (2*dv);
    }
    
};

// a uniformly twisty flow
class basicflow : public curlnoise {
private:
    perlin pnoise;
    
public:
    basicflow() : curlnoise() {};
    
    vec3 potential(float x, float y, float z) {
        vec3 p;
        // some seriously magic numbers
        // but also seriously, we sample the noise at offsets because
        // in theory they should not be correlated
        p[0] = pnoise(x, y, z);
        p[1] = pnoise(y+31.416f,  z-47.853f, x+12.793f);
        p[2] = pnoise(z-233.145f, x-113.408f, y-185.31f);
        
        p[0] += 0.25 * time * 0.001 * pnoise(x*2, y*2, z*2);
        p[1] += 0.25 * time * 0.001 * pnoise((y+31.416f)*2,  (z-47.853f)*2, (x+12.793f)*2);
        p[2] += 0.25 * time * 0.001 * pnoise((z-233.145f)*2, (x-113.408f)*2, (y-185.31f)*2);
        
        p[0] += 0.0625 * pnoise( x*4,            y*4,            z*4);
        p[1] += 0.0625 * pnoise((y+31.416f)*4,  (z-47.853f)*4,  (x+12.793f)*4);
        p[2] += 0.0625 * pnoise((z-233.145f)*4, (x-113.408f)*4, (y-185.31f)*4);
        
        return p;
    }
};

// some random utilities
class randutils {	
public:
	static std::mt19937 engine;
	static std::uniform_real_distribution<float> uniform_bipolar;
	static std::uniform_real_distribution<float> canonical;
	static std::normal_distribution<float> normal;

	/** returns a random point inside a sphere. Subsequent points may very well overlap */
	static point3 sphere_point( point3 &origin, float radius ) {
		// randomly generate spherical coordinates
		float phi = uniform_bipolar(engine) * M_PI;
		float theta = uniform_bipolar(engine) * M_PI;
		float rad = canonical(engine) * radius;
		// convert to cartesian
		point3 p;
		p[0] = rad * sin(theta) * cos(phi);
		p[1] = rad * sin(theta) * sin(phi);
		p[2] = rad * cos(theta);

		p = p + origin; // translate appropriately

		return p;
	}

	/** returns a random point inside a cube, normally distributed about the given origin.
	    No reason why they may not overlap */
	static point3 cube_normal_point( point3 &origin, float radius ) {
		point3 p;
		p[0] = normal(engine) * radius;
		p[1] = normal(engine) * radius;
		p[2] = normal(engine) * radius;
		p = p + origin;
		return p;
	}
};

std::random_device d;
std::mt19937 randutils::engine = std::mt19937(d());
std::uniform_real_distribution<float> randutils::uniform_bipolar = std::uniform_real_distribution<float>(-1.0f,1.0f);
std::normal_distribution<float> randutils::normal = std::normal_distribution<float>();
std::uniform_real_distribution<float> randutils::canonical = std::uniform_real_distribution<float>(0.0f,1.0f);

#endif
