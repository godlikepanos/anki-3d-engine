#ifndef PATH_H
#define PATH_H

#include "Resource.h"
#include "Math.h"


/// Path @ref Resource resource
class Path: public Resource
{
	public:
		Vec<Vec3> positions; ///< AKA translations
		Vec<Mat3> rotations;
		Vec<float>  scales;
		float         step;

		Path() {}
		~Path() {}
		bool load(const char* filename);
};


#endif
