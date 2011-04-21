#ifndef GL_STATE_MACHINE_H
#define GL_STATE_MACHINE_H

#include <GL/glew.h>
#include "Assert.h"
#include "Singleton.h"


/// Access the GL state machine.
/// This class saves us from calling the GL functions
class GlStateMachine
{
	public:
		GlStateMachine() {sync();}

		/// Sync the local members with the opengl ones
		void sync();

		/// @name Set the Fixed Function Pipeline, Call the OpenGL functions only when needed
		/// @{
		bool isDepthTestEnabled() const {return getFlag(GL_DEPTH_TEST, depthTestEnabledFlag);}
		void setDepthTestEnabled(bool enable) {setFlag(GL_DEPTH_TEST, enable, depthTestEnabledFlag);}

		bool isBlendingEnabled() const {return getFlag(GL_BLEND, blendingEnabledFlag);}
		void setBlendingEnabled(bool enable) {setFlag(GL_BLEND, enable, blendingEnabledFlag);}

		void useShaderProg(GLuint id);
		/// @}

	private:
		/// @name The GL state
		/// @{
		bool depthTestEnabledFlag;
		bool blendingEnabledFlag;
		GLuint sProgGlId;
		/// @}

		static bool getFlag(GLenum glFlag, bool myFlag);
		static void setFlag(GLenum glFlag, bool enable, bool& myFlag);

		static GLuint getCurrentProgramGlId();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline bool GlStateMachine::getFlag(GLenum glFlag, bool myFlag)
{
	ASSERT(glIsEnabled(glFlag) == myFlag);
	return myFlag;
}


inline void GlStateMachine::setFlag(GLenum glFlag, bool enable, bool& myFlag)
{
	ASSERT(glIsEnabled(glFlag) == myFlag);

	if(enable != myFlag)
	{
		if(enable)
		{
			glEnable(glFlag);
		}
		else
		{
			glDisable(glFlag);
		}
		myFlag = enable;
	}
}


inline void GlStateMachine::useShaderProg(GLuint id)
{
	ASSERT(getCurrentProgramGlId() == sProgGlId);

	if(sProgGlId != id)
	{
		glUseProgram(id);
		sProgGlId = id;
	}
}


inline GLuint GlStateMachine::getCurrentProgramGlId()
{
	int i;
	glGetIntegerv(GL_CURRENT_PROGRAM, &i);
	return i;
}


//======================================================================================================================
// Other                                                                                                               =
//======================================================================================================================

typedef Singleton<GlStateMachine> GlStateMachineSingleton; ///< Make the GlStateMachine singleton class


#endif
