// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/OcclusionQueryImpl.h>

namespace anki
{

void OcclusionQueryImpl::init()
{
	glGenQueries(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);
}

void OcclusionQueryImpl::begin()
{
	ANKI_ASSERT(isCreated());
	glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, m_glName);
}

void OcclusionQueryImpl::end()
{
	ANKI_ASSERT(isCreated());
	glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
}

OcclusionQueryResult OcclusionQueryImpl::getResult() const
{
	ANKI_ASSERT(isCreated());
	OcclusionQueryResult result = OcclusionQueryResult::NOT_AVAILABLE;
	GLuint params;
	glGetQueryObjectuiv(m_glName, GL_QUERY_RESULT_AVAILABLE, &params);

	if(params != 0)
	{
		glGetQueryObjectuiv(m_glName, GL_QUERY_RESULT, &params);
		result = (params == 1) ? OcclusionQueryResult::VISIBLE : OcclusionQueryResult::NOT_VISIBLE;
	}

	return result;
}

} // end namespace anki
