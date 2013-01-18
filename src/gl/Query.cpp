#include "anki/gl/Query.h"
#include "anki/util/Exception.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Query::Query(GLenum q)
{
	create(q);
}

//==============================================================================
Query::~Query()
{
	destroy();
}

//==============================================================================
Query& Query::operator=(Query&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));
	question = b.question;
	return *this;
}

//==============================================================================
void Query::create(const GLenum q)
{
	ANKI_ASSERT(!isCreated());
	crateNonSharable();
	glGenQueries(1, &glId);
	question = q;

	if(glId == 0)
	{
		throw ANKI_EXCEPTION("glGenQueries() failed");
	}
}

//==============================================================================
void Query::destroy()
{
	if(isCreated())
	{
		checkNonSharable();
		glDeleteQueries(1, &glId);
		glId = 0;
	}
}

//==============================================================================
void Query::begin()
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();

	glBeginQuery(question, glId);
}

//==============================================================================
void Query::end()
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();

	glEndQuery(question);
}

//==============================================================================
GLuint64 Query::getResult()
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();

#if ANKI_GL == ANKI_GL_DESKTOP
	GLuint64 result;
	glGetQueryObjectui64v(glId, GL_QUERY_RESULT, &result);
#else
	GLuint result;
	glGetQueryObjectuiv(glId, GL_QUERY_RESULT, &result);
#endif
	return result;
}

//==============================================================================
GLuint64 Query::getResultNoWait(Bool& finished)
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();

	GLuint resi;
	glGetQueryObjectuiv(glId, GL_QUERY_RESULT_AVAILABLE, &resi);

#if ANKI_GL == ANKI_GL_DESKTOP
	GLuint64 result;
#else
	GLuint result;
#endif
	if(resi)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
		glGetQueryObjectui64v(glId, GL_QUERY_RESULT, &result);
#else
		glGetQueryObjectuiv(glId, GL_QUERY_RESULT, &result);
#endif
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
