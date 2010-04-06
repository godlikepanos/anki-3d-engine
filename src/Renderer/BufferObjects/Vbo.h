#ifndef _VBO_H_
#define _VBO_H_

#include "Common.h"
#include "BufferObject.h"

/**
 * @brief This is a wrapper for Vertex Buffer Objects to prevent us from making idiotic errors
 */
class Vbo: public BufferObject
{
	protected:
		GLenum usage; ///< GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW

	public:
		GLenum getBufferUsage() const { DEBUG_ERR(glId==0); return usage; }

		/**
		 * Creates a new VBO with the given params and checks if everything went OK
		 * @param target_ It should be: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER only!!!!!
		 * @param sizeInBytes The size of the buffer that we will allocate in bytes
		 * @param dataPtr Points to the data buffer to copy to the VGA memory. Put NULL if you want just to allocate memory
		 * @param usage_ It should be: GL_STREAM_DRAW or GL_STATIC_DRAW or GL_DYNAMIC_DRAW only!!!!!!!!!
		 */
		void create( GLenum target_, uint sizeInBytes, const void* dataPtr, GLenum usage_ )
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
		 * Unbinds all VBOs, meaning both GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER targets
		 */
		static void unbindAllTargets()
		{
			glBindBufferARB( GL_ARRAY_BUFFER, 0 );
			glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}
};

#endif
