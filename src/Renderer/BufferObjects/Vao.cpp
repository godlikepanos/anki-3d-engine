#include "Vao.h"
#include "Vbo.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Vao::VboInfo::VboInfo(const Vbo* vbo_, const ShaderProg::AttribVar* attribVar_, GLint size_, GLenum type_,
                      GLboolean normalized_, GLsizei stride_, const GLvoid* pointer_):
	vbo(vbo_),
	attribVar(attribVar_),
	size(size_),
	type(type_),
	normalized(normalized_),
	stride(stride_),
	pointer(pointer_)
{
	RASSERT_THROW_EXCEPTION(vbo == NULL);
	RASSERT_THROW_EXCEPTION(attribVar == NULL);
	RASSERT_THROW_EXCEPTION(size < 1);
}


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
inline void Vao::create(Vec<VboInfo> arrayBufferVbosInfo, const Vbo* elementArrayBufferVbo)
{
	RASSERT_THROW_EXCEPTION(isCreated());
	glGenVertexArrays(1, &glId);
	bind();

	// Attach the GL_ARRAY_BUFFER VBOs
	Vec<VboInfo>::iterator info = arrayBufferVbosInfo.begin();
	for(; info != arrayBufferVbosInfo.end(); info++)
	{
		RASSERT_THROW_EXCEPTION(info->vbo == NULL);
		RASSERT_THROW_EXCEPTION(info->attribVar == NULL);

		if(info->vbo->getBufferTarget() != GL_ARRAY_BUFFER)
		{
			throw EXCEPTION("Only GL_ARRAY_BUFFER is accepted");
		}

		info->vbo->bind();
		glVertexAttribPointer(info->attribVar->getLoc(), info->size, info->type, info->normalized,
		                      info->stride, info->pointer);
		glEnableVertexAttribArray(info->attribVar->getLoc());
		info->vbo->unbind();
	}

	// Attach the GL_ELEMENT_ARRAY_BUFFER VBO
	if(elementArrayBufferVbo != NULL)
	{
		if(elementArrayBufferVbo->getBufferTarget() != GL_ELEMENT_ARRAY_BUFFER)
		{
			throw EXCEPTION("Only GL_ELEMENT_ARRAY_BUFFER is accepted");
		}

		elementArrayBufferVbo->bind();
	}

	unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}
