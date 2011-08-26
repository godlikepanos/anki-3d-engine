/// @file
/// For Internal use in math lib

#ifndef M_COMMON_H
#define M_COMMON_H

#include "util/StdTypes.h"
#include "util/Accessors.h"
#if defined(MATH_INTEL_SIMD)
#	include <smmintrin.h>
#endif
#include <iostream>
#include <boost/array.hpp>


// Forward delcs
namespace m {

class Vec2;
class Vec3;
class Vec4;
class Quat;
class Euler;
class Axisang;
class Mat3;
class Mat4;
class Transform;

}


#endif
