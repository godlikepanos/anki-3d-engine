// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Shader.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Shader implementation.
class ShaderImpl final : public Shader
{
public:
	GrDynamicArray<U8> m_binary;
	ShaderReflection m_reflection;

	ShaderImpl(CString name)
		: Shader(name)
	{
	}

	~ShaderImpl();

	Error init(const ShaderInitInfo& init);
};
/// @}

} // end namespace anki
