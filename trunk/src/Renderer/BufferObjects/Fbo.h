#ifndef FBO_H
#define FBO_H

#include <GL/glew.h>
#include "Assert.h"
#include "StdTypes.h"
#include "Exception.h"


/// The class is actually a wrapper to avoid common mistakes
class Fbo
{
	public:
		/// Constructor
		Fbo(): glId(0) {}

		/// Destructor
		~Fbo();

		uint getGlId() const;

		/// Binds FBO
		void bind() const;

		/// Unbinds the FBO. Actually unbinds all FBOs
		static void unbind();

		/// Checks the status of an initialized FBO and if fails throw exception
		/// @exception Exception
		void checkIfGood() const;

		/// Set the number of color attachements of the FBO
		void setNumOfColorAttachements(uint num) const;

		/// Returns the GL id of the current attached FBO
		/// @return Returns the GL id of the current attached FBO
		static uint getCurrentFbo();

		/// Creates a new FBO
		void create();

		/// Destroy it
		void destroy();

	private:
		uint glId; ///< OpenGL identification

		bool isCreated() const {return glId != 0;}
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline void Fbo::create()
{
	ASSERT(!isCreated());
	glGenFramebuffers(1, &glId);
}


inline uint Fbo::getGlId() const
{
	ASSERT(!isCreated());
	return glId;
}


inline Fbo::~Fbo()
{
	if(isCreated())
	{
		destroy();
	}
}


inline void Fbo::bind() const
{
	ASSERT(isCreated());
	glBindFramebuffer(GL_FRAMEBUFFER, glId);
}


inline void Fbo::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


inline void Fbo::destroy()
{
	ASSERT(isCreated());
	glDeleteFramebuffers(1, &glId);
}


#endif
