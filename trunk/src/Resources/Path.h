#ifndef _PATH_H_
#define _PATH_H_

#include "Common.h"
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
		void unload() { points.clear(); }
};


#endif
