/// @file
/// For Internal use in math lib

#ifndef M_COMMON_H
#define M_COMMON_H

#include "Util/StdTypes.h"
#include "Util/Accessors.h"
#if defined(MATH_INTEL_SIMD)
	#include <smmintrin.h>
#endif
#include <iostream>
#include <boost/array.hpp>


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
