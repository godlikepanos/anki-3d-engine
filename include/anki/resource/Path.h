#ifndef ANKI_RESOURCE_PATH_H
#define ANKI_RESOURCE_PATH_H

#include "anki/math/Math.h"


namespace anki {


/// Path @ref Resource resource
class Path
{
	public:
		Vector<Vec3> positions; ///< AKA translations
		Vector<Mat3> rotations;
		Vector<float>  scales;
		float         step;

		Path() {}
		~Path() {}
		bool load(const char* filename);
};


} // end namespace


#endif
