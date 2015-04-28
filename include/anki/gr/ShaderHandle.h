// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_SHADER_HANDLE_H
#define ANKI_GR_SHADER_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Shader handle.
class ShaderHandle: public GrHandle<ShaderImpl>
{
public:
	using Base = GrHandle<ShaderImpl>;

	ShaderHandle();

	~ShaderHandle();

	/// Create shader program.
	ANKI_USE_RESULT Error create(GrManager* manager, 
		ShaderType shaderType, const void* source, PtrSize sourceSize);
};
/// @}

} // end namespace anki

#endif

