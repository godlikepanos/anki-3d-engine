#ifndef GL_STATE_MACHINE_H
#define GL_STATE_MACHINE_H

#include <GL/glew.h>
#include <boost/unordered_map.hpp>
#include "Assert.h"


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
		void enable(GLenum flag, bool enable = true);
		void disable(GLenum flag) {enable(flag, false);}
		bool isEnabled(GLenum flag);

		void useShaderProg(GLuint id);
		/// @}

	private:
		/// @name The GL state
		/// @{
		GLuint sProgGlId;

		boost::unordered_map<GLenum, bool> flags;
		static GLenum flagEnums[];
		/// @}

		static GLuint getCurrentProgramGlId();
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

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


#endif
