// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_SHADER_HANDLE_H
#define ANKI_GR_SHADER_HANDLE_H

#include "anki/gr/ShaderCommon.h"
#include "anki/gr/GrPtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Shader.
class ShaderPtr: public GrPtr<ShaderImpl>
{
public:
	using Base = GrPtr<ShaderImpl>;

	ShaderPtr();

	~ShaderPtr();

	/// Create shader program.
	void create(GrManager* manager,
		ShaderType shaderType, const void* source, PtrSize sourceSize);
};
/// @}

} // end namespace anki

#endif

