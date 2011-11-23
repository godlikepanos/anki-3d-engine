#ifndef ANKI_RESOURCE_MATERIAL_COMMON_H
#define ANKI_RESOURCE_MATERIAL_COMMON_H

#include "anki/resource/PassLevelKey.h"
#include <boost/array.hpp>


namespace anki {


/// Given a pair of pass and level it returns a pointer to a shader program
typedef PassLevelHashMap<const class ShaderProgram*>::Type
	PassLevelToShaderProgramHashMap;


} // end namespace


#endif
