#ifndef ANKI_GL_QUERY_H
#define ANKI_GL_QUERY_H

#include "anki/gl/Ogl.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup OpenGL
///

/// Query object
class Query
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// @param q One of GL_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED, 
	///          GL_TIME_ELAPSED
	Query(GLenum q);

	~Query();
	/// @}

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
	GLuint glId;
	GLenum question;
};
/// @}

} // end namespace anki

#endif
