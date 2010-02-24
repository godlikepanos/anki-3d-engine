#ifndef _VBO_H_
#define _VBO_H_

#include <GL/glew.h>
#include <GL/gl.h>
#include "common.h"

/// This is a wrapper for Vertex Buffer Objects to prevent us from making idiotic errors
class vbo_t
{
	protected:
		uint glId; ///< The OpenGL id of the VBO
		// the below vars can be extracted by quering OpenGL but I suppose keeping them here is faster
		GLenum target;
		GLenum usage;

	public:
		vbo_t(): glId(0) {}
		virtual ~vbo_t() { Delete(); }
		uint   getGlId() const { DEBUG_ERR(glId==0); return glId; }
		GLenum GetBufferTarget() const { DEBUG_ERR(glId==0); return target; }
		GLenum GetBufferUsage() const { DEBUG_ERR(glId==0); return usage; }

		/**
		 * Creates a new VBO with the given params and checks if everything went OK
		 * @param target_ It should be: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER only!!!!!
		 * @param size_in_bytes The size of the buffer that we will allocate in bytes
		 * @param data_ptr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		 * @param usage_ It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		 */
		void Create( GLenum target_, uint size_in_bytes, const void* data_ptr, GLenum usage_ )
		{
			DEBUG_ERR( glId!=0 ); // VBO allready initialized
			DEBUG_ERR( target_!=GL_ARRAY_BUFFER && target_!=GL_ELEMENT_ARRAY_BUFFER ); // unacceptable target_
			DEBUG_ERR( usage_!=GL_STREAM_DRAW && usage_!=GL_STATIC_DRAW && usage_!=GL_DYNAMIC_DRAW ); // unacceptable usage_
			DEBUG_ERR( size_in_bytes < 1 ); // unacceptable size

			usage = usage_;
			target = target_;

			glGenBuffers( 1, &glId );
			Bind();
			glBufferData( target, size_in_bytes, data_ptr, usage ); // allocate memory and copy data from data_ptr to the VBO. If data_ptr is NULL just allocate

			// make a check
			int buffer_size = 0;
			glGetBufferParameteriv( target, GL_BUFFER_SIZE, &buffer_size );
			if( size_in_bytes != (uint)buffer_size )
			{
				Delete();
				ERROR( "Data size mismatch" );
				return;
			}

			Unbind();
		}

		/**
		 * Deletes the VBO
		 */
		void Delete()
		{
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glDeleteBuffers( 1, &glId );
			glId = 0;
		}

		/**
		 * bind the VBO
		 */
		void Bind() const
		{
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glBindBuffer( target, glId );
		}

		/**
		 * Unbinds only the targets that have the same target as this
		 */
		void Unbind() const
		{
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glBindBuffer( target, 0 );
		}

		/**
		 * Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER targets
		 */
		static void UnbindAllTargets()
		{
			glBindBufferARB( GL_ARRAY_BUFFER, 0 );
			glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}
};

#endif
