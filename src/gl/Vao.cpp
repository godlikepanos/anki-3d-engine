#include "anki/gl/Vao.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Exception.h"
#include "anki/gl/ShaderProgram.h"

namespace anki {

//==============================================================================

std::atomic<uint32_t> Vao::counter(0);

//==============================================================================
Vao::~Vao()
{
	if(isCreated())
	{
		destroy();
	}
}

//==============================================================================
void Vao::attachArrayBufferVbo(const VaoDescriptor& descr)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(descr.vbo != nullptr);
	ANKI_ASSERT(descr.attribVar != nullptr);
	ANKI_ASSERT(descr.size != 0);
	ANKI_ASSERT(descr.type != 0);

	ANKI_ASSERT(descr.vbo->getBufferTarget() == GL_ARRAY_BUFFER
		&& "Only GL_ARRAY_BUFFER is accepted");

	bind();
	descr.vbo->bind();
	glVertexAttribPointer(descr.attribVar->getLocation(), descr.size, 
		descr.type, descr.normalized, descr.stride, descr.pointer);
	glEnableVertexAttribArray(descr.attribVar->getLocation());
	descr.vbo->unbind();
	unbind();

	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void Vao::attachElementArrayBufferVbo(const Vbo& vbo)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(vbo.getBufferTarget() == GL_ELEMENT_ARRAY_BUFFER
		&& "Only GL_ELEMENT_ARRAY_BUFFER is accepted");

	bind();
	vbo.bind();
	unbind();
	ANKI_CHECK_GL_ERROR();
}

} // end namespace
