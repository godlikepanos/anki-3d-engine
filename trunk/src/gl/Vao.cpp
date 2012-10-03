#include "anki/gl/Vao.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Exception.h"
#include "anki/gl/ShaderProgram.h"

namespace anki {

//==============================================================================

thread_local const Vao* Vao::current = nullptr;

//==============================================================================
Vao::~Vao()
{
	if(isCreated())
	{
		destroy();
	}
}

//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo, GLuint attribVarLocation,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid* pointer)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(vbo.getBufferTarget() == GL_ARRAY_BUFFER
		&& "Only GL_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo.isCreated());

	bind();
	vbo.bind();
	glEnableVertexAttribArray(attribVarLocation);
	glVertexAttribPointer(attribVarLocation, size, type, normalized,
		stride, pointer);
	vbo.unbind();
	unbind();

	ANKI_CHECK_GL_ERROR();

#if !defined(NDEBUG)
	++attachments;
#endif
}

//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo,
	const ShaderProgramAttributeVariable& attribVar,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid* pointer)
{
	attachArrayBufferVbo(vbo, attribVar.getLocation(), size, type, normalized,
		stride, pointer);
}

//==============================================================================
void Vao::attachElementArrayBufferVbo(const Vbo& vbo)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(vbo.getBufferTarget() == GL_ELEMENT_ARRAY_BUFFER
		&& "Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo.isCreated());

	bind();
	vbo.bind();
	unbind();
	ANKI_CHECK_GL_ERROR();

#if !defined(NDEBUG)
	++attachments;
#endif
}

} // end namespace anki
