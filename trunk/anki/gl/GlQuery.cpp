#include "anki/gl/Query.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <GL/glew.h>


namespace anki {


//==============================================================================
// QueryImpl                                                                   =
//==============================================================================

/// Query implementation
struct QueryImpl
{
	GLuint glId;
	GLenum question;
};


//==============================================================================
// Query                                                                       =
//==============================================================================

//==============================================================================
Query::Query(QueryQuestion q)
{
	impl.reset(new QueryImpl);

	// question
	switch(q)
	{
		case QQ_SAMPLES_PASSED:
			impl->question = GL_SAMPLES_PASSED;
			break;
		case QQ_ANY_SAMPLES_PASSED:
			impl->question = GL_ANY_SAMPLES_PASSED;
			break;
		case QQ_TIME_ELAPSED:
			impl->question = GL_TIME_ELAPSED;
			break;
		default:
			ANKI_ASSERT(0);
	};

	// glId
	glGenQueries(1, &impl->glId);
	if(impl->glId == 0)
	{
		throw ANKI_EXCEPTION("Query generation failed");
	}
}


//==============================================================================
Query::~Query()
{
	glDeleteQueries(1, &impl->glId);
}


//==============================================================================
void Query::beginQuery()
{
	glBeginQuery(impl->question, impl->glId);
}


//==============================================================================
void Query::endQuery()
{
	glEndQuery(impl->question);
}


//==============================================================================
uint64_t Query::getResult()
{
	GLuint64 result;
	glGetQueryObjectui64v(impl->glId, GL_QUERY_RESULT, &result);
	return result;
}


//==============================================================================
uint64_t Query::getResultNoWait(bool& finished)
{
	GLuint resi;
	glGetQueryObjectuiv(impl->glId, GL_QUERY_RESULT_AVAILABLE, &resi);

	GLuint64 result;
	if(resi)
	{
		glGetQueryObjectui64v(impl->glId, GL_QUERY_RESULT, &result);
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
