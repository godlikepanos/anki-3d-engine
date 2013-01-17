#ifndef ANKI_GL_VAO_H
#define ANKI_GL_VAO_H

#include "anki/gl/GlException.h"
#include "anki/gl/GlObject.h"

namespace anki {

class ShaderProgramAttributeVariable;
class Vbo;

/// @addtogroup OpenGL
/// @{

/// Vertex array object. Non-copyable to avoid instantiating it in the stack
class Vao: public GlObjectContextNonSharable
{
public:
	typedef GlObjectContextNonSharable Base;

	/// @name Constructors/Destructor
	/// @{

	/// Default
	Vao()
	{}

	/// Move
	Vao(Vao&& b)
	{
		*this = std::move(b);
	}

	/// Destroy VAO from the OpenGL context
	~Vao()
	{
		destroy();
	}
	/// @}

	/// @name Operators
	/// @{

	/// Move
	Vao& operator=(Vao&& b)
	{
		crateNonSharable();
		destroy();

		Base::operator=(std::forward<Base>(b));
		attachments = b.attachments;
		b.attachments = 0;
		return *this;
	}
	/// @}

	/// Create
	void create()
	{
		ANKI_ASSERT(!isCreated());
		crateNonSharable();
		glGenVertexArrays(1, &glId);
		ANKI_CHECK_GL_ERROR();
	}

	/// Destroy
	void destroy();

	/// Attach an array buffer VBO. See @link
	/// http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribPointer.xml
	/// @endlink
	/// @param vbo The VBO to attach
	/// @param attribVar For the shader attribute location
	/// @param size Specifies the number of components per generic vertex
	///        attribute. Must be 1, 2, 3, 4
	/// @param type GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT etc
	/// @param normalized Specifies whether fixed-point data values should
	///        be normalized
	/// @param stride Specifies the byte offset between consecutive generic
	///        vertex attributes
	/// @param offset Specifies a offset of the first component of the
	///        first generic vertex attribute in the array
	void attachArrayBufferVbo(
	    const Vbo* vbo,
	    const ShaderProgramAttributeVariable& attribVar,
	    const PtrSize size,
	    const GLenum type,
	    const Bool normalized,
	    const PtrSize stride,
	    const PtrSize offset);

	/// Attach an array buffer VBO. See @link
	/// http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribPointer.xml
	/// @endlink
	/// @param vbo The VBO to attach
	/// @param attribVarLocation Shader attribute location
	/// @param size Specifies the number of components per generic vertex
	///        attribute. Must be 1, 2, 3, 4
	/// @param type GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT etc
	/// @param normalized Specifies whether fixed-point data values should
	///        be normalized
	/// @param stride Specifies the byte offset between consecutive generic
	///        vertex attributes
	/// @param pointer Specifies a offset of the first component of the
	///        first generic vertex attribute in the array
	void attachArrayBufferVbo(
	    const Vbo* vbo,
	    const GLint attribVarLocation,
	    const PtrSize size,
	    const GLenum type,
	    const Bool normalized,
	    const PtrSize stride,
	    const PtrSize offset);

	U32 getAttachmentsCount() const
	{
		ANKI_ASSERT(isCreated());
		return attachments;
	}

	/// Attach an element array buffer VBO
	void attachElementArrayBufferVbo(const Vbo* vbo);

	/// Bind it
	void bind() const
	{
		ANKI_ASSERT(isCreated());
		checkNonSharable();
		if(current != this)
		{
			glBindVertexArray(glId);
			current = this;
		}
		ANKI_ASSERT(getCurrentVertexArrayBinding() == glId);
	}

	/// Unbind all VAOs
	void unbind() const
	{
		ANKI_ASSERT(isCreated());
		checkNonSharable();
		if(current == this)
		{
			glBindVertexArray(0);
			current = nullptr;
		}
	}

private:
	static const Vao* current;
	U32 attachments = 0;

	static GLuint getCurrentVertexArrayBinding()
	{
		GLint x;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &x);
		return (GLuint)x;
	}
};
/// @}

} // end namespace anki

#endif
