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
#include <vector>
#include <functional>

// some attempt at a parallel-for, that might help
// speed things up
namespace concurrent_tools {
    static constexpr unsigned leaf_coarseness = 5000;
    
    
    
    static inline void __for(unsigned begin,
                             unsigned end,
                             std::function<void(unsigned)> f) {
        for (unsigned i = begin; i < end; i++)
            f(i);
    }
    
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
    
    // not recursive version
    static inline void itparallel_for(unsigned begin,
                                      unsigned end,
                                      std::function<void(unsigned)> f,
                                      unsigned group_size=leaf_coarseness) {
        unsigned num = (end-begin)/group_size + 1;
        std::vector<std::thread> v;
        unsigned b,e;
        for (unsigned i = 0; i < num; i++) {
            b = begin + i*group_size;
            e = ((b + group_size) < end)? b+group_size : end;
            v.push_back( std::thread( __for,b,e,f ) );
        }
        // wait till they are all done
        // this is possibly irritating,
        // maybe we are waiting on them in a weird order
        for (unsigned i = 0; i < num; i++) {
            v[i].join();
        }
    }
}


#endif
