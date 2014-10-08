//
//  interpolation.h
//  noisepart
//
//  Created by Paul Francis Cunninghame Mathews on 7/10/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#ifndef noisepart_interpolation_h
#define noisepart_interpolation_h

static float lerp( const float a, const float b, const float c ) {
    return a + c*(b-a);
}


#endif
