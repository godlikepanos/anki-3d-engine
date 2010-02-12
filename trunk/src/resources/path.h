#ifndef _PATH_H_
#define _PATH_H_

#include "common.h"
#include "resource.h"
#include "gmath.h"


/// Path
class path_t: public resource_t
{
	public:
		vec_t<vec3_t> positions; ///< AKA translations
		vec_t<mat3_t> rotations;
		vec_t<float>  scales;
		float         step;

		path_t() {}
		~path_t() {}
		bool Load( const char* filename );
		void Unload() { points.clear(); }
};


#endif
