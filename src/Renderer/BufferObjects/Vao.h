#ifndef VAO_H
#define VAO_H

#include <GL/glew.h>
#include <limits>
#include "Exception.h"
#include "StdTypes.h"
#include "ShaderProg.h"


class Vbo;


/// Vertex array object
class Vao
{
	public:
		/// This structs contains information for VAO bind into the VAO. Used by the @ref create method. See
		/// glVertexAttribPointer for the interpretation of the member variables
		struct VboInfo
		{
			const Vbo* vbo;
			const ShaderProg::AttribVar* attribVar;
			GLint size;
			GLenum type;
			GLboolean normalized;
			GLsizei stride;
			const GLvoid* pointer;

			/// The one and only constructor
			VboInfo(const Vbo* vbo_, const ShaderProg::AttribVar* attribVar_, GLint size_, GLenum type_,
              GLboolean normalized_, GLsizei stride_, const GLvoid* pointer_);

      ~VboInfo() {}
		};

		/// Default constructor
		Vao(): glId(std::numeric_limits<uint>::max()) {}

		/// Destructor
		~Vao();

		/// Safe accessor. Throws exception if not created
		/// @exception Exception
		uint getGlId() const;

		/// Create the VAO. Throws exception if already created. It requires an C-style array with size so it can be fast
		/// @param[in] arrayBufferVbosInfo An array with information per VBO. The VBOs are GL_ARRAY_BUFFER
		/// @param[in] arrayBufferVbosInfoNum The size of the arrayBufferVbosInfo array
		/// @param[in] elementArrayBufferVbo The GL_ELEMENT_ARRAY_BUFFER VBO. If NULL then ignored
		/// @exception Exception
		void create(const VboInfo arrayBufferVbosInfo[], uint arrayBufferVbosInfoNum, const Vbo* elementArrayBufferVbo);

		/// Bind it. Throws exception if not created
		/// @exception Exception
		void bind() const;

		/// Unbind all VAOs
		static void unbind() {glBindVertexArray(0);}

		/// Destroy it. Throws exception if not created
		void destroy();

		/// Return true if the VAO is already created
		bool isCreated() const {return glId != std::numeric_limits<uint>::max();}

	private:
		uint glId; ///< The OpenGL id
};


inline uint Vao::getGlId() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	return glId;
}


inline void Vao::bind() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glBindVertexArray(glId);
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
	RASSERT_THROW_EXCEPTION(!isCreated());
	glDeleteVertexArrays(1, &glId);
}


#endif
