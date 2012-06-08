#ifndef ANKI_GL_VAO_H
#define ANKI_GL_VAO_H

#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/gl/GlException.h"
#include "anki/gl/Gl.h"
#include <atomic>

namespace anki {

class ShaderProgramAttributeVariable;
class Vbo;

/// Vertex array object. Non-copyable to avoid instantiating it in the stack
class Vao: public NonCopyable
{
public:
	/// See @link
	/// http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribPointer.xml
	/// @endlink
	struct VaoDescriptor
	{
		const Vbo* vbo = nullptr;
		const ShaderProgramAttributeVariable* attribVar = nullptr;
		/// Specifies the number of components per generic vertex attribute. 
		/// Must be 1, 2, 3, 4
		GLint size = 0;
		/// GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT etc
		GLenum type = 0;
		/// Specifies whether fixed-point data values should be normalized
		GLboolean normalized = GL_FALSE;
		/// Specifies the byte offset between consecutive generic vertex 
		/// attributes
		GLsizei stride = 0;
		/// Specifies a offset of the first component of the first generic 
		/// vertex attribute in the array
		const GLvoid* pointer = nullptr;
	};

	/// @name Constructors/Destructor
	/// @{

	/// Default
	Vao()
	{}

	/// Destroy VAO from the OpenGL context
	~Vao();
	/// @}

	/// @name Accessors
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}

	uint32_t getUuid() const
	{
		return uuid;
	}
	/// @}

	/// Create
	void create()
	{
		ANKI_ASSERT(!isCreated());
		glGenVertexArrays(1, &glId);
		ANKI_CHECK_GL_ERROR();
		uuid = counter.fetch_add(1, std::memory_order_relaxed);
	}

	/// Destroy
	void destroy()
	{
		ANKI_ASSERT(isCreated());
		glDeleteVertexArrays(1, &glId);
	}

	/// Attach an array buffer VBO
	void attachArrayBufferVbo(const VaoDescriptor& descr);

	/// Attach an element array buffer VBO
	void attachElementArrayBufferVbo(const Vbo& vbo);

	/// Bind it
	void bind() const
	{
		glBindVertexArray(glId);
	}

	/// Unbind all VAOs
	static void unbind()
	{
		glBindVertexArray(0);
	}

private:
	static std::atomic<uint32_t> counter;

	GLuint glId = 0; ///< The OpenGL id
	uint32_t uuid;

	bool isCreated() const
	{
		return glId != 0;
	}
};

} // end namespace

#endif
