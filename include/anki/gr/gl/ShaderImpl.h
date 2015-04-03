// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_SHADER_IMPL_H
#define ANKI_GR_GL_SHADER_IMPL_H

#include "anki/gr/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Shader program. It only contains a single shader and it can be combined 
/// with other programs in a program pipiline.
class ShaderImpl: public GlObject
{
	friend class GlProgramVariable;
	friend class GlProgramBlock;

public:
	using Base = GlObject;

	ShaderImpl(GrManager* manager)
	:	Base(manager)
	{}

	~ShaderImpl()
	{
		destroy();
	}

	/// Create the shader.
	/// @param shaderType The type of the shader in the program
	/// @param source The shader's source
	ANKI_USE_RESULT Error create(
		ShaderType shaderType, 
		const CString& source);

	GLenum getGlType() const
	{
		ANKI_ASSERT(isCreated());
		return m_glType;
	}

	ShaderType getType() const
	{
		ANKI_ASSERT(isCreated());
		return m_type;
	}

private:
	GLenum m_glType = 0;
	ShaderType m_type;

	void destroy();

	void handleError(String& src);
};
/// @}

} // end namespace anki

#endif

