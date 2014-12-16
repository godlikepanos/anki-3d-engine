// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlOcclusionQuery.h"

namespace anki {

//==============================================================================
Error GlOcclusionQuery::create()
{
	glGenQueries(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
	return ErrorCode::NONE;
}

//==============================================================================
void GlOcclusionQuery::begin()
{
	ANKI_ASSERT(isCreated());
	glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_glName);
}

//==============================================================================
void GlOcclusionQuery::end()
{
	ANKI_ASSERT(isCreated());
	glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
}

//==============================================================================
GlOcclusionQuery::Result GlOcclusionQuery::getResult()
{
	ANKI_ASSERT(isCreated());
	Result result = Result::NOT_AVAILABLE;
	GLuint params;
	glGetQueryObjectuiv(m_glName, GL_QUERY_RESULT_AVAILABLE, &params);

	if(params != 0)
	{
		glGetQueryObjectuiv(m_glName, GL_QUERY_RESULT, &params);

		result = (params == 1) ? Result::VISIBLE : Result::NOT_VISIBLE;
	}

	return result;
}

//==============================================================================
void GlOcclusionQuery::destroy()
{
	if(m_glName != 0)
	{
		glDeleteQueries(1, &m_glName);
		m_glName = 0;
	}
}

} // end namespace anki

