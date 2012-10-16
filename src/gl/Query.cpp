#include "anki/gl/Query.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Query::Query(GLenum q)
{
	question = q;

	// glId
	glGenQueries(1, &glId);
	ANKI_ASSERT(glId != 0);
}

//==============================================================================
Query::~Query()
{
	glDeleteQueries(1, &glId);
}

//==============================================================================
void Query::begin()
{
	glBeginQuery(question, glId);
}

//==============================================================================
void Query::end()
{
	glEndQuery(question);
}

//==============================================================================
GLuint64 Query::getResult()
{
	GLuint64 result;
	glGetQueryObjectui64v(glId, GL_QUERY_RESULT, &result);
	return result;
}

//==============================================================================
GLuint64 Query::getResultNoWait(Bool& finished)
{
	GLuint resi;
	glGetQueryObjectuiv(glId, GL_QUERY_RESULT_AVAILABLE, &resi);

	GLuint64 result;
	if(resi)
	{
		glGetQueryObjectui64v(glId, GL_QUERY_RESULT, &result);
		finished = true;
	}
	else
	{
		finished = false;
		result = 0;
	}

	return result;
}

} // end namespace anki
