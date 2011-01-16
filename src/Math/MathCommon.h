/// @file
/// For Internal use in math lib

#ifndef MATH_COMMON_H
#define MATH_COMMON_H

#include <iostream>
#include <boost/array.hpp>
#include "StdTypes.h"
#if defined(MATH_INTEL_SIMD)
	#include <smmintrin.h>
#endif


// Forward delcs
namespace M {

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
