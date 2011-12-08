#include "anki/gl/Vao.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Exception.h"
#include "anki/resource/ShaderProgram.h"


namespace anki {


//==============================================================================
Vao::~Vao()
{
	if(isCreated())
	{
		destroy();
	}
}


//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo, uint attribVarLocation,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid* pointer)
{
	ANKI_ASSERT(isCreated());
	if(vbo.getBufferTarget() != GL_ARRAY_BUFFER)
	{
		throw ANKI_EXCEPTION("Only GL_ARRAY_BUFFER is accepted");
	}

	ANKI_CHECK_GL_ERROR();

	bind();
	vbo.bind();
	glVertexAttribPointer(attribVarLocation, size, type, normalized,
		stride, pointer);
	glEnableVertexAttribArray(attribVarLocation);
	vbo.unbind();
	unbind();

	ANKI_CHECK_GL_ERROR();
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
	if(vbo.getBufferTarget() != GL_ELEMENT_ARRAY_BUFFER)
	{
		throw ANKI_EXCEPTION("Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	}

	bind();
	vbo.bind();
	unbind();
	ANKI_CHECK_GL_ERROR();
}


} // end namespace
