#ifndef GL_EXCEPTION_H
#define GL_EXCEPTION_H


/// The function throws an exception if there is an OpenGL error. Use it with
/// the ON_GL_FAIL_THROW_EXCEPTION macro
void glConditionalThrowException(const char* file, int line, const char* func);


#define ON_GL_FAIL_THROW_EXCEPTION() \
	glConditionalThrowException(__FILE__, __LINE__, __func__)


#endif
