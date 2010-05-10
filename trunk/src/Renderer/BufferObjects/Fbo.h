#ifndef _FBO_H_
#define _FBO_H_

#include <GL/glew.h>
#include "Common.h"


/// The class is created as a wrapper to avoid common mistakes
class Fbo
{
	PROPERTY_R( uint, glId, getGlId ) ///< OpenGL idendification

	public:
		Fbo(): glId(0) {}

		/// Creates a new FBO
		void create()
		{
			DEBUG_ERR( glId != 0 ); // FBO already initialized
			glGenFramebuffers( 1, &glId );
		}

		/// Binds FBO
		void bind() const
		{
			DEBUG_ERR( glId == 0 );  // FBO unitialized
			glBindFramebuffer( GL_FRAMEBUFFER, glId );
		}

		/// Unbinds the FBO. Actualy unbinds all FBOs
		static void unbind() { glBindFramebuffer( GL_FRAMEBUFFER, 0 ); }

		/**
		 * Checks the status of an initialized FBO
		 * @return True if FBO is ok and false if not
		 */
		bool isGood() const
		{
			DEBUG_ERR( glId == 0 );  // FBO unitialized
			DEBUG_ERR( getCurrentFbo() != glId ); // another FBO is binded

			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		/// Set the number of color attachements of the FBO
		void setNumOfColorAttachements( uint num ) const
		{
			DEBUG_ERR( glId == 0 );  // FBO unitialized
			DEBUG_ERR( getCurrentFbo() != glId ); // another FBO is binded

			if( num == 0 )
			{
				glDrawBuffer( GL_NONE );
				glReadBuffer( GL_NONE );
			}
			else
			{
				static GLenum colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
				                                     GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
				glDrawBuffers( num, colorAttachments );
			}
		}

		/**
		 * Returns the GL id of the current attached FBO
		 * @return Returns the GL id of the current attached FBO
		 */
		static uint getCurrentFbo()
		{
			int fboGlId;
			glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fboGlId );
			return (uint)fboGlId;
		}
};

#endif
