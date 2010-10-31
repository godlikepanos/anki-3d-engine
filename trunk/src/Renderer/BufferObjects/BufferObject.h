#ifndef BUFFER_OBJECT_H
#define BUFFER_OBJECT_H

#include <GL/glew.h>
#include <limits>
#include "Exception.h"
#include "StdTypes.h"


/// A wrapper for OpenGL buffer objects (vertex arrays, texture buffers etc) to prevent us from making idiotic errors
class BufferObject
{
	public:
		BufferObject(): glId(std::numeric_limits<uint>::max()) {}
		virtual ~BufferObject();

		/// Safe accessor. Throws exception if BO is not created
		/// @return The OpenGL ID of the buffer
		/// @exception Exception
		uint getGlId() const;

		/// Safe accessor. Throws exception if BO is not created
		/// @return OpenGL target
		/// @exception Exception
		GLenum getBufferTarget() const;

		/// Safe accessor. Throws exception if BO is not created
		/// @return GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW
		/// @exception Exception
		GLenum getBufferUsage() const;

		/// Checks if BO is created
		/// @return True if BO is already created
		bool isCreated() const throw() {return glId != std::numeric_limits<uint>::max();}

		/// Creates a new BO with the given parameters and checks if everything went OK. Throws exception if fails
		/// @param target Depends on the BO
		/// @param sizeInBytes The size of the buffer that we will allocate in bytes
		/// @param dataPtr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		/// @param usage It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		/// @exception Exception
		void create(GLenum target, uint sizeInBytes, const void* dataPtr, GLenum usage);

		/// Bind BO. Throws exception if BO is not created
		/// @exception Exception
		void bind() const;

		/// Unbind BO. Throws exception if BO is not created
		/// @exception Exception
		void unbind() const;

	protected:
		uint glId; ///< The OpenGL id of the BO

		/// Used in glBindBuffer(target, glId) and its for easy access so we wont have to query the GL driver. Its the type
		/// of the buffer eg GL_TEXTURE_BUFFER or GL_ELEMENT_ARRAY_BUFFER etc
		GLenum target;

		GLenum usage; ///< GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW

	private:
		/// Delete the BO
		/// @exception Exception
		void deleteBuff();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline BufferObject::~BufferObject()
{
	if(isCreated())
	{
		deleteBuff();
	}
}


inline uint BufferObject::getGlId() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	return glId;
}


inline GLenum BufferObject::getBufferTarget() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	return target;
}


inline GLenum BufferObject::getBufferUsage() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	return usage;
}


inline void BufferObject::bind() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glBindBuffer(target, glId);
}


inline void BufferObject::unbind() const
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glBindBuffer(target, 0);
}


inline void BufferObject::deleteBuff()
{
	RASSERT_THROW_EXCEPTION(!isCreated());
	glDeleteBuffers(1, &glId);
	glId = 0;
}


#endif
