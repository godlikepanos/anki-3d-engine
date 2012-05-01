#ifndef ANKI_GL_QUERY_H
#define ANKI_GL_QUERY_H

#include <cstdint>
#include <GL/glew.h>


namespace anki {


class QueryImpl;


/// XXX
class Query
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// @param q One of GL_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED, 
	///        GL_TIME_ELAPSED
	Query(GLenum q);

	~Query();
	/// @}

	/// Start
	void beginQuery();

	/// End
	void endQuery();

	/// Get results
	/// @note Waits for operations to finish
	uint64_t getResult();

	/// Get results. Doesn't Wait for operations to finish. If @a finished is 
	/// false then the return value is irrelevant
	uint64_t getResultNoWait(bool& finished);

private:
	GLuint glId;
	GLenum question;
};


} // end namespace anki


#endif
