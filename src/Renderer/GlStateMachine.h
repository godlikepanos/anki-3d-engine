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
		/// @}

	private:
		bool depthTestEnabledFlag;
		bool blendingEnabledFlag;

		static bool getFlag(GLenum glFlag, bool myFlag);
		static void setFlag(GLenum glFlag, bool enable, bool& myFlag);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline void GlStateMachine::sync()
{
	depthTestEnabledFlag = glIsEnabled(GL_DEPTH_TEST);
	blendingEnabledFlag = glIsEnabled(GL_BLEND);
}


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


//======================================================================================================================
// Other                                                                                                               =
//======================================================================================================================

typedef Singleton<GlStateMachine> GlStateMachineSingleton; ///< Make the GlStateMachine singleton class


#endif
