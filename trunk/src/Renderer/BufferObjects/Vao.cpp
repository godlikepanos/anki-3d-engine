#include "Vao.h"
#include "Vbo.h"


//======================================================================================================================
// attachArrayBufferVbo                                                                                                =
//======================================================================================================================
void Vao::attachArrayBufferVbo(const Vbo& vbo, const ShaderProg::AttribVar& attribVar, GLint size, GLenum type,
		                           GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	if(vbo.getBufferTarget() != GL_ARRAY_BUFFER)
	{
		throw EXCEPTION("Only GL_ARRAY_BUFFER is accepted");
	}

	bind();
	vbo.bind();
	glVertexAttribPointer(attribVar.getLoc(), size, type, normalized, stride, pointer);
	glEnableVertexAttribArray(attribVar.getLoc());
	unbind();

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// attachElementArrayBufferVbo                                                                                         =
//======================================================================================================================
void Vao::attachElementArrayBufferVbo(const Vbo& vbo)
{
	if(vbo.getBufferTarget() != GL_ELEMENT_ARRAY_BUFFER)
	{
		throw EXCEPTION("Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	}

	bind();
	vbo.bind();
	unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}
