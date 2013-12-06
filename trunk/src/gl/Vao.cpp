#include "anki/gl/Vao.h"
#include "anki/gl/BufferObject.h"
#include "anki/util/Exception.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================

const Vao* Vao::current = nullptr;

//==============================================================================
void Vao::destroy()
{
	checkNonSharable();

	if(isCreated())
	{
		unbind();
		glDeleteVertexArrays(1, &glId);
		glId = 0;
		attachments = 0;
	}
}

//==============================================================================
void Vao::attachArrayBufferVbo(const BufferObject* vbo, 
	const GLint attribVarLocation,
	const PtrSize size, const GLenum type, const Bool normalized, 
	const PtrSize stride, const PtrSize offset)
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();
	ANKI_ASSERT(vbo->getTarget() == GL_ARRAY_BUFFER
		&& "Only GL_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo->isCreated());

	bind();
	vbo->bind();
	glEnableVertexAttribArray(attribVarLocation);
	glVertexAttribPointer(attribVarLocation, size, type, normalized,
		stride, reinterpret_cast<const GLvoid*>(offset));
	vbo->unbind();
	unbind();

	ANKI_CHECK_GL_ERROR();

	++attachments;
}

//==============================================================================
void Vao::attachArrayBufferVbo(const BufferObject* vbo,
	const ShaderProgramAttributeVariable& attribVar,
	const PtrSize size, const GLenum type, const Bool normalized, 
	const PtrSize stride, const PtrSize offset)
{
	attachArrayBufferVbo(vbo, attribVar.getLocation(), size, type, normalized,
		stride, offset);
}

//==============================================================================
void Vao::attachElementArrayBufferVbo(const BufferObject* vbo)
{
	ANKI_ASSERT(isCreated());
	checkNonSharable();
	ANKI_ASSERT(vbo->getTarget() == GL_ELEMENT_ARRAY_BUFFER
		&& "Only GL_ELEMENT_ARRAY_BUFFER is accepted");
	ANKI_ASSERT(vbo->isCreated());

	bind();
	vbo->bind();
	unbind();
	ANKI_CHECK_GL_ERROR();

	++attachments;
}

} // end namespace anki
