// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/ShaderProgram.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram
{
public:
	ShaderProgramImpl(CString name)
		: ShaderProgram(name)
	{
	}

	~ShaderProgramImpl();

	Error init(const ShaderProgramInitInfo& inf);
};
/// @}

} // end namespace anki
