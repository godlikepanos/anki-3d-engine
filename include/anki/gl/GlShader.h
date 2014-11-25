// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_SHADER_H
#define ANKI_GL_GL_SHADER_H

#include "anki/gl/GlObject.h"
#include "anki/util/Dictionary.h"
#include "anki/util/DArray.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Shader program. It only contains a single shader and it can be combined 
/// with other programs in a program pipiline.
class GlShader: public GlObject
{
	friend class GlProgramVariable;
	friend class GlProgramBlock;

public:
	using Base = GlObject;

	template<typename T>
	using DArray = DArray<T, GlAllocator<T>>;

	GlShader() = default;

	~GlShader()
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
	GLenum m_type;

	void destroy();

	/// Query the shader for blocks
	ANKI_USE_RESULT Error initBlocksOfType(GLenum programInterface, 
		U count, U index, char*& namesPtr, U& namesLen);

	ANKI_USE_RESULT Error handleError(GlAllocator<U8>& alloc, String& src);
};

/// @}

} // end namespace anki

#endif

