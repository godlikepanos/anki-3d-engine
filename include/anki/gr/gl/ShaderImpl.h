// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_SHADER_IMPL_H
#define ANKI_GR_GL_SHADER_IMPL_H

#include "anki/gr/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Shader program. It only contains a single shader and it can be combined 
/// with other programs in a program pipiline.
class ShaderImpl: public GlObject
{
	friend class GlProgramVariable;
	friend class GlProgramBlock;

public:
	using Base = GlObject;

	ShaderImpl() = default;

	~ShaderImpl()
	{
		destroy();
	}

	/// Create the shader.
	/// @param shaderType The type of the shader in the program
	/// @param source The shader's source
	/// @param alloc The allocator to be used for internally
	ANKI_USE_RESULT Error create(
		GLenum shaderType, 
		const CString& source, 
		GlAllocator<U8>& alloc, 
		const CString& cacheDir);

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated());
		return m_type;
	}

private:
	GLenum m_type = 0;

	void destroy();

	ANKI_USE_RESULT Error handleError(GlAllocator<U8>& alloc, String& src);
};

/// @}

} // end namespace anki

#endif

