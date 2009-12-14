#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include <math.h>
#include "common.h"
#include "gmath.h"
#include "engine_class.h"

// vertex_t
class vertex_t
{
	public:
		vec3_t coords;
		vec3_t normal;
};

// triangle_t
class triangle_t
{
	public:
		uint   vert_ids[3]; // an array with the vertex indexes in the mesh class
		vec3_t normal;
};

























#endif
