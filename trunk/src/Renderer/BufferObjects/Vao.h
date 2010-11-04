#ifndef VAO_H
#define VAO_H

#include <GL/glew.h>
#include "Exception.h"
#include "StdTypes.h"
#include "ShaderProg.h"
#include "Object.h"
#include "Properties.h"
#include "GlException.h"


class Vbo;


/// Vertex array object
class Vao: public Object
{
	PROPERTY_R(uint, glId, getGlId) ///< The OpenGL id

	public:
		/// Default constructor
		Vao(Object* parent = NULL);

		/// Destroy VAO fro the OpenGL context
		~Vao() {glDeleteVertexArrays(1, &glId);}

		/// @todo
		void attachArrayBufferVbo(const Vbo& vbo, const ShaderProg::AttribVar& attribVar, GLint size, GLenum type,
		                          GLboolean normalized, GLsizei stride, const GLvoid* pointer);

		/// @todo
		void attachElementArrayBufferVbo(const Vbo& vbo);

		/// Bind it
		void bind() const {glBindVertexArray(glId);}

		/// Unbind all VAOs
		static void unbind() {glBindVertexArray(0);}
};


inline Vao::Vao(Object* parent):
	Object(parent)
{
	glGenVertexArrays(1, &glId);
	ON_GL_FAIL_THROW_EXCEPTION();
}


#endif
