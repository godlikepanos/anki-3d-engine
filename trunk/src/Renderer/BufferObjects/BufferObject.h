#ifndef _BUFFEROBJECT_H_
#define _BUFFEROBJECT_H_

#include "Common.h"
#include <GL/glew.h>

/**
 * @brief A wrapper for OpenGL buffer objects ( vertex arrays, texture buffers etc ) to prevent us from making idiotic errors
 */
class BufferObject
{
	protected:
		uint glId; ///< The OpenGL id of the BO
		GLenum target; ///< Used in glBindBuffer( target, glId ) and for easy access

	public:
		BufferObject(): glId(0) {}
		virtual ~BufferObject() { if(glId!=0) deleteBuff(); }

		/**
		 * Accessor
		 * @return The OpenGL ID of the buffer
		 */
		uint getGlId() const
		{
			DEBUG_ERR( glId==0 );
			return glId;
		}

		GLenum getBufferTarget() const
		{
			DEBUG_ERR( glId==0 );
			return target;
		}

		void bind() const
		{
			DEBUG_ERR( glId==0 );
			glBindBuffer( target, glId );
		}

		void unbind() const
		{
			DEBUG_ERR( glId==0 );
			glBindBuffer( target, 0 );
		}

		void deleteBuff()
		{
			DEBUG_ERR( glId==0 );
			glDeleteBuffers( 1, &glId );
			glId = 0;
		}
};

#endif
