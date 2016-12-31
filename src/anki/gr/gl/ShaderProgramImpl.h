// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

static inline void deletePrograms(GLsizei n, const GLuint* progs)
{
	ANKI_ASSERT(n == 1);
	ANKI_ASSERT(progs);
	glDeleteProgram(*progs);
}

/// Shader program implementation.
class ShaderProgramImpl : public GlObject
{
public:
	ShaderProgramImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~ShaderProgramImpl()
	{
		destroyDeferred(deletePrograms);
	}

	ANKI_USE_RESULT Error initGraphics(
		ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag);
	ANKI_USE_RESULT Error initCompute(ShaderPtr comp);

private:
	ANKI_USE_RESULT Error link(GLuint vert, GLuint frag);
};
/// @}

} // end namespace anki
