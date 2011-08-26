#include "Vao.h"
#include "Vbo.h"
#include "util/Exception.h"
#include "rsrc/ShaderProgramAttributeVariable.h"


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Vao::~Vao()
{
	if(isCreated())
	{
		destroy();
	}
}


//==============================================================================
// attachArrayBufferVbo                                                        =
//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo, uint attribVarLocation,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid* pointer)
{
	ASSERT(isCreated());
	if(vbo.getBufferTarget() != GL_ARRAY_BUFFER)
	{
		throw EXCEPTION("Only GL_ARRAY_BUFFER is accepted");
	}

	ON_GL_FAIL_THROW_EXCEPTION();

	bind();
	vbo.bind();
	glVertexAttribPointer(attribVarLocation, size, type, normalized,
		stride, pointer);
	glEnableVertexAttribArray(attribVarLocation);
	vbo.unbind();
	unbind();

	ON_GL_FAIL_THROW_EXCEPTION();
}


//==============================================================================
// attachArrayBufferVbo                                                        =
//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo,
	const ShaderProgramAttributeVariable& attribVar,
	GLint size, GLenum type, GLboolean normalized, GLsizei stride,
	const GLvoid* pointer)
{
	attachArrayBufferVbo(vbo, attribVar.getLoc(), size, type, normalized,
		stride, pointer);
}


//==============================================================================
// attachElementArrayBufferVbo                                                 =
//==============================================================================
void Vao::attachElementArrayBufferVbo(const Vbo& vbo)
{
	ASSERT(isCreated());
	if(vbo.getBufferTarget() != GL_ELEMENT_ARRAY_BUFFER)
	{
		throw EXCEPTION("Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	}

	bind();
	vbo.bind();
	unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}
