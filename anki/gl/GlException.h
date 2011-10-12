#ifndef ANKI_GL_GL_EXCEPTION_H
#define ANKI_GL_GL_EXCEPTION_H


namespace anki {


/// The function throws an exception if there is an OpenGL error. Use it with
/// the ANKI_CHECK_GL_ERROR macro
void glConditionalThrowException(const char* file, int line, const char* func);


#define ANKI_CHECK_GL_ERROR() \
	glConditionalThrowException(__FILE__, __LINE__, __func__)


} // end namespace


#endif
