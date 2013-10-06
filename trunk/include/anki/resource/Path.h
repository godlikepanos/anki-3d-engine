#ifndef ANKI_RESOURCE_PATH_H
#define ANKI_RESOURCE_PATH_H

#include "anki/Math.h"

namespace anki {

/// Path @ref Resource resource
class Path
{
	public:
		Vector<Vec3> positions; ///< AKA translations
		Vector<Quat> rotations;
		Vector<F32> scales;
		F32 step;

		Path() {}
		~Path() {}
		bool load(const char* filename);
};

} // end namespace anki

#endif
