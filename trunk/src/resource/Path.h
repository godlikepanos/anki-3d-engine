#ifndef PATH_H
#define PATH_H

#include "math/Math.h"


/// Path @ref Resource resource
class Path
{
	public:
		std::vector<Vec3> positions; ///< AKA translations
		std::vector<Mat3> rotations;
		std::vector<float>  scales;
		float         step;

		Path() {}
		~Path() {}
		bool load(const char* filename);
};


#endif
