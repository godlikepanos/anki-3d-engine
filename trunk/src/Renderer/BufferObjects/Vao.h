#ifndef VAO_H
#define VAO_H

#include <GL/glew.h>
#include "StdTypes.h"
#include "ShaderProg.h"
#include "Object.h"
#include "GlException.h"


class Vbo;


/// Vertex array object
class Vao
{
	public:
		/// Default constructor
		Vao(): glId(0) {}

		/// Destroy VAO from the OpenGL context
		~Vao();

		/// Accessor
		uint getGlId() const;

		/// Create
		void create();

		/// Destroy
		void destroy();

		/// Attach an array buffer VBO. See @link http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribPointer.xml
		/// @endlink
		/// @param vbo The VBO to attach
		/// @param attribVar For the shader attribute location
		/// @param size Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4
		/// @param type GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT etc
		/// @param normalized Specifies whether fixed-point data values should be normalized
		/// @param stride Specifies the byte offset between consecutive generic vertex attributes
		/// @param pointer Specifies a offset of the first component of the first generic vertex attribute in the array
		void attachArrayBufferVbo(const Vbo& vbo, const SProgAttribVar& attribVar, GLint size, GLenum type,
		                          GLboolean normalized, GLsizei stride, const GLvoid* pointer);

		/// Attach an array buffer VBO. See @link http://www.opengl.org/sdk/docs/man3/xhtml/glVertexAttribPointer.xml
		/// @endlink
		/// @param vbo The VBO to attach
		/// @param attribVarLocation Shader attribute location
		/// @param size Specifies the number of components per generic vertex attribute. Must be 1, 2, 3, 4
		/// @param type GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT etc
		/// @param normalized Specifies whether fixed-point data values should be normalized
		/// @param stride Specifies the byte offset between consecutive generic vertex attributes
		/// @param pointer Specifies a offset of the first component of the first generic vertex attribute in the array
		void attachArrayBufferVbo(const Vbo& vbo, uint attribVarLocation, GLint size, GLenum type,
		                          GLboolean normalized, GLsizei stride, const GLvoid* pointer);

		/// Attach an element array buffer VBO
		void attachElementArrayBufferVbo(const Vbo& vbo);

		/// Bind it
		void bind() const {glBindVertexArray(glId);}

		/// Unbind all VAOs
		static void unbind() {glBindVertexArray(0);}

	private:
		uint glId; ///< The OpenGL id

		bool isCreated() const {return glId != 0;}
};


inline void Vao::create()
{
	ASSERT(!isCreated());
	glGenVertexArrays(1, &glId);
	ON_GL_FAIL_THROW_EXCEPTION();
}


inline Vao::~Vao()
{
	if(isCreated())
	{
		destroy();
	}
}


inline void Vao::destroy()
{
	ASSERT(isCreated());
	glDeleteVertexArrays(1, &glId);
}


inline uint Vao::getGlId() const
{
	ASSERT(isCreated());
	return glId;
}


#endif
