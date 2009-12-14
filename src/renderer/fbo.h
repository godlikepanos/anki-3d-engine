#ifndef _FBO_H_
#define _FBO_H_

#include "common.h"
#include "renderer.h"


/// The class is created as a wrapper to avoid common mistakes
class fbo_t
{
	PROPERTY_R( uint, gl_id, GetGLID ) ///< OpenGL idendification

	public:
		fbo_t(): gl_id(0) {}

		/// Creates a new FBO
		void Create()
		{
			DEBUG_ERR( gl_id != 0 ); // FBO allready initialized
			glGenFramebuffersEXT( 1, &gl_id );
		}

		/// Binds FBO
		void Bind() const
		{
			DEBUG_ERR( gl_id == 0 );  // FBO unitialized
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gl_id );
		}

		/// Unbinds the FBO. Actualy unbinds all FBOs
		static void Unbind() { glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ); }

		/**
		 * Checks the status of an initialized FBO
		 * @return True if FBO is ok and false if not
		 */
		bool CheckStatus() const
		{
			DEBUG_ERR( gl_id == 0 );  // FBO unitialized
			DEBUG_ERR( GetCurrentFBO() != gl_id ); // another FBO is binded

			return glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT;
		}

		/// Set the number of color attachements of the FBO
		void SetNumOfColorAttachements( uint num ) const
		{
			DEBUG_ERR( gl_id == 0 );  // FBO unitialized
			DEBUG_ERR( GetCurrentFBO() != gl_id ); // another FBO is binded

			if( num == 0 )
			{
				glDrawBuffer( GL_NONE );
				glReadBuffer( GL_NONE );
			}
			else
			{
				GLenum color_attachments[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT,
				                               GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT, GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT };
				glDrawBuffers( num, color_attachments );
			}
		}

		/**
		 * Returns the GL id of the current attached FBO
		 * @return Returns the GL id of the current attached FBO
		 */
		static uint GetCurrentFBO()
		{
			int fbo_gl_id;
			glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &fbo_gl_id );
			return (uint)fbo_gl_id;
		}
};

#endif
