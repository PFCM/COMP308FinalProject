//
//  concurrencytools.h
//  noisepart
//
//  Created by Paul Francis Cunninghame Mathews on 13/10/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#ifndef noisepart_concurrencytools_h
#define noisepart_concurrencytools_h
#include <thread>
#include <functional>

// some attempt at a parallel-for, that might help
// speed things up
namespace concurrent_tools {
    static constexpr unsigned leaf_coarseness = 5000;
    
    static inline void parallel_for(unsigned begin,
                                    unsigned end,
                                    std::function<void(unsigned)> f,
                                    unsigned coarse=leaf_coarseness) {
        if (end-begin <= leaf_coarseness) { // do the work
            for (unsigned i = begin; i < end; i++)
                f(i);
        } else {
            std::thread t1(parallel_for,begin, begin+(end-begin)/2, f, coarse);
            std::thread t2(parallel_for,begin+(end-begin)/2, end, f,coarse);
            t1.join();
            t2.join();
        }
    }

}


#endif
