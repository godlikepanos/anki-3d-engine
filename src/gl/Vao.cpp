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
void Vao::attachArrayBufferVbo(const Vbo* vbo, const GLint attribVarLocation,
	const PtrSize size, const GLenum type, const Bool normalized, 
	const PtrSize stride, const PtrSize offset)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(vbo.getBufferTarget() == GL_ARRAY_BUFFER
		&& "Only GL_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo.isCreated());
	checkNonSharable();

	bind();
	vbo->bind();
	glEnableVertexAttribArray(attribVarLocation);
	glVertexAttribPointer(attribVarLocation, size, type, normalized,
		stride, reinterpret_cast<const GLvoid*>(offset));
	vbo.unbind();
	unbind();

	ANKI_CHECK_GL_ERROR();

	++attachments;
}

//==============================================================================
void Vao::attachArrayBufferVbo(const Vbo* vbo,
	const ShaderProgramAttributeVariable& attribVar,
	const PtrSize size, const GLenum type, const Bool normalized, 
	const PtrSize stride, const PtrSize offset)
{
	attachArrayBufferVbo(vbo, attribVar.getLocation(), size, type, normalized,
		stride, offset);
}

//==============================================================================
void Vao::attachElementArrayBufferVbo(const Vbo* vbo)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(vbo.getBufferTarget() == GL_ELEMENT_ARRAY_BUFFER
		&& "Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo.isCreated());
	checkNonSharable();

	bind();
	vbo.bind();
	unbind();
	ANKI_CHECK_GL_ERROR();

	++attachments;
}

} // end namespace anki
