// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/OcclusionQueryImpl.h"

namespace anki {

//==============================================================================
Error OcclusionQueryImpl::create(GlOcclusionQueryResultBit condRenderingBit)
{
	glGenQueries(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
	m_condRenderingBit = condRenderingBit;
	return ErrorCode::NONE;
}

//==============================================================================
void OcclusionQueryImpl::begin()
{
	ANKI_ASSERT(isCreated());
	glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_glName);
}

//==============================================================================
void OcclusionQueryImpl::end()
{
	ANKI_ASSERT(isCreated());
	glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
}

//==============================================================================
OcclusionQueryImpl::Result OcclusionQueryImpl::getResult() const
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
void OcclusionQueryImpl::destroy()
{
	if(m_glName != 0)
	{
		glDeleteQueries(1, &m_glName);
		m_glName = 0;
	}
}

} // end namespace anki

