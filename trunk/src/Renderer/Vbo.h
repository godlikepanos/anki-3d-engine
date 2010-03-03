#ifndef _VBO_H_
#define _VBO_H_

#include <GL/glew.h>
#include <GL/gl.h>
#include "Common.h"

/// This is a wrapper for Vertex Buffer Objects to prevent us from making idiotic errors
class Vbo
{
	protected:
		uint glId; ///< The OpenGL id of the VBO
		// the below vars can be extracted by quering OpenGL but I suppose keeping them here is faster
		GLenum target;
		GLenum usage;

	public:
		Vbo(): glId(0) {}
		virtual ~Vbo() { deleteBuff(); }
		uint   getGlId() const { DEBUG_ERR(glId==0); return glId; }
		GLenum getBufferTarget() const { DEBUG_ERR(glId==0); return target; }
		GLenum getBufferUsage() const { DEBUG_ERR(glId==0); return usage; }

		/**
		 * Creates a new VBO with the given params and checks if everything went OK
		 * @param target_ It should be: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER only!!!!!
		 * @param size_in_bytes The size of the buffer that we will allocate in bytes
		 * @param data_ptr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		 * @param usage_ It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		 */
		void Create( GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_ )
		{
			DEBUG_ERR( glId!=0 ); // VBO already initialized
			DEBUG_ERR( target_!=GL_ARRAY_BUFFER && target_!=GL_ELEMENT_ARRAY_BUFFER ); // unacceptable target_
			DEBUG_ERR( usage_!=GL_STREAM_DRAW && usage_!=GL_STATIC_DRAW && usage_!=GL_DYNAMIC_DRAW ); // unacceptable usage_
			DEBUG_ERR( sizeInBytes < 1 ); // unacceptable size

			usage = usage_;
			target = target_;

			glGenBuffers( 1, &glId );
			bind();
			glBufferData( target, sizeInBytes, dataPtr, usage ); // allocate memory and copy data from data_ptr to the VBO. If data_ptr is NULL just allocate

			// make a check
			int bufferSize = 0;
			glGetBufferParameteriv( target, GL_BUFFER_SIZE, &bufferSize );
			if( sizeInBytes != (uint)bufferSize )
			{
				deleteBuff();
				ERROR( "Data size mismatch" );
				return;
			}

			unbind();
		}

		/**
		 * Deletes the VBO
		 */
		void deleteBuff()
		{
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glDeleteBuffers( 1, &glId );
			glId = 0;
		}

		/**
		 * Bind the VBO
		 */
		void bind() const
		{
			if( glId==0 )
				PRINT( "-" );
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glBindBuffer( target, glId );
		}

		/**
		 * Unbinds only the targets that have the same target as this
		 */
		void unbind() const
		{
			DEBUG_ERR( glId==0 ); // VBO unitialized
			glBindBuffer( target, 0 );
		}

		/**
		 * Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER targets
		 */
		static void unbindAllTargets()
		{
			glBindBufferARB( GL_ARRAY_BUFFER, 0 );
			glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}
};

#endif
