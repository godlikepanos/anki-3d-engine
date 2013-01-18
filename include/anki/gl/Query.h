#ifndef ANKI_GL_QUERY_H
#define ANKI_GL_QUERY_H

#include "anki/gl/GlObject.h"

namespace anki {

/// @addtogroup OpenGL
/// @{

/// Query object
class Query: public GlObjectContextNonSharable
{
public:
	typedef GlObjectContextNonSharable Base;

	/// @name Constructors/Destructor
	/// @{

	/// @param q One of GL_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED, 
	///          GL_TIME_ELAPSED
	Query(GLenum q);

	/// Move
	Query(Query&& b)
	{
		*this = std::move(b);
	}

	~Query();
	/// @}

	/// Move
	Query& operator=(Query&& b);

	/// Start
	void begin();

	/// End
	void end();

	/// Get results. Waits for operations to finish
	GLuint64 getResult();

	/// Get results. Doesn't Wait for operations to finish. If @a finished is
	/// false then the return value is irrelevant
	GLuint64 getResultNoWait(Bool& finished);

private:
	GLenum question;

	void create(const GLenum q);

	void destroy();
};

/// @}

} // end namespace anki

#endif
