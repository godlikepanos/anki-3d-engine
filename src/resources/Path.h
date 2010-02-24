#ifndef _PATH_H_
#define _PATH_H_

#include "common.h"
#include "Resource.h"
#include "gmath.h"


/// Path resource
class Path: public Resource
{
	public:
		Vec<vec3_t> positions; ///< AKA translations
		Vec<mat3_t> rotations;
		Vec<float>  scales;
		float         step;

		Path() {}
		~Path() {}
		bool load( const char* filename );
		void unload() { points.clear(); }
};


#endif
