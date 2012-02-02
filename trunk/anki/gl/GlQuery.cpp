#include "anki/gl/Query.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <GL/glew.h>


namespace anki {


//==============================================================================
// QueryImpl                                                                   =
//==============================================================================

/// XXX
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


} // end namespace anki
