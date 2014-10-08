//-----------------------------------------
// COMP 308 Mid-term project
// Paul Mathews, 300190240, 2014
//
// This is a point,using templates for the type
// of number used and the dimensionality
// which should mean that the provided biz
// works for 2,3 or homogeneous
//------------------------------------------

#ifndef POINTH
#define POINTH

#include <cmath>
#include <iostream>

template < class NumType=float,
std::size_t dimensions=2 >
class point {
    
    NumType data[dimensions];
public:
    enum {
        X,Y,Z,W
    };
    
    
    // constructors
    point() {
        for (unsigned i =0; i < dimensions; i++) {
            data[i] =0;
        }
    };
    ~point() {
        //delete [] data;
    };
    
    point(const point& other) {
        for (int i = 0; i < dimensions; i++) {
            data[i] = other.data[i];
        }
    }
    
    point& operator=(const point& other) { // should check dimensions are the same
        for (int i = 0; i < dimensions; i++) {
            data[i] = other.data[i];
        }
        return *this;
    };
    NumType length() const {
        NumType t = 0;
        // TODO: ignore last for homogeneous
        for (int i = 0; i < dimensions; i++) { // hopefully this gets unrolled by the compiler
            t += data[i] * data[i];
        }
        return sqrt(t);
    };
    point normalise() const {
        NumType l = length();
        point<NumType, dimensions> p;
        p = *this;
        for (int i = 0; i < dimensions; i++) {
            p.data[i] /= l;
        }
        return p;
    };
    
    NumType &operator[] (std::size_t which) {
        return data[which];
    };
    
    const NumType &operator[] (std::size_t which) const {
        return const_cast<NumType&>(data[which]);
    }
    
    point &operator+=(const point &other) {
        for (unsigned i = 0; i < dimensions; i++) {
            data[i] += other.data[i];
        }
        return *this;
    }
    
    friend point operator+(const point& lhs, const point& rhs) {
        point<NumType, dimensions> p;
        for (int i = 0; i < dimensions; i++) {
            p.data[i] = lhs.data[i] + rhs.data[i];
        }
        return p;
    };
    friend point operator-(const point& lhs, const point& rhs) {
        point<NumType, dimensions> p;
        for (int i = 0; i < dimensions; i++) {
            p.data[i] = lhs.data[i] - rhs.data[i];
        }
        return p;
    };
    
    friend point operator*(const point &lhs, const NumType rhs) {
        point<NumType, dimensions> p;
        for (int i = 0; i < dimensions; i++) {
            p.data[i] = lhs.data[i] * rhs;
        }
        return p;
    }
    friend point operator*(const NumType lhs, const point &rhs) {
        return rhs*lhs;
    }
    
    
    static NumType dotproduct(const point& a, const point& b) {
        NumType l = 0;
        for (int i = 0; i < dimensions; i++) {
            l += a[i] * b[i];
        }
        return l;
    };
    
    static point<NumType, 3> crossproduct( const point<NumType, 3>& a, const point<NumType, 3>& b ) {
        point<NumType, 3> result;
        result[0] = a[1]*b[2] - a[2]*b[1];
        result[1] = a[2]*b[0] - a[0]*b[2];
        result[2] = a[0]*b[1] - a[1]*b[0];
        return result;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const point<NumType, dimensions> p) {
        os << "(";
        for (int i = 0; i < dimensions-1; i++)
            os << p[i] << ",";
        os << p[dimensions-1] << ")";
        return os;
    }
};

point<float, 3> crossproduct(const point<float, 3>& a, const point<float, 3>& b) {
    point<float, 3> cross;
    cross[0] = a[1] * b[2] - a[2] * b[1];
    cross[1] = a[2] * b[0] - a[0] * b[2];
    cross[2] = a[0] * b[1] - a[1] * b[0];
    return cross;
}

typedef point<float, 3> point3;
typedef point<float, 2> point2;
typedef point<int, 3> point3i;
typedef point3 vec3;

#endif
















