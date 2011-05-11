#include "GlStateMachine.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================

GLenum GlStateMachine::flagEnums[] =
{
	GL_DEPTH_TEST,
	GL_BLEND,
	0
};


//======================================================================================================================
// enable                                                                                                              =
//======================================================================================================================
void GlStateMachine::enable(GLenum glFlag, bool enable)
{
	ASSERT(flags.find(glFlag) != flags.end());
	bool state = flags[glFlag];
	ASSERT(glIsEnabled(glFlag) == state);

	if(enable != state)
	{
		if(enable)
		{
			glEnable(glFlag);
		}
		else
		{
			glDisable(glFlag);
		}
		flags[glFlag] = enable;
	}
}


//======================================================================================================================
// isEnabled                                                                                                           =
//======================================================================================================================
bool GlStateMachine::isEnabled(GLenum glFlag)
{
	ASSERT(flags.find(glFlag) != flags.end());
	bool state = flags[glFlag];
	ASSERT(glIsEnabled(glFlag) == state);
	return state;
}


//======================================================================================================================
// sync                                                                                                                =
//======================================================================================================================
void GlStateMachine::sync()
{
	sProgGlId = getCurrentProgramGlId();

	// Set flags
	GLenum* flagEnum = &flagEnums[0];
	while(*flagEnum != 0)
	{
		flags[*flagEnum] = glIsEnabled(*flagEnum);
		++flagEnum;
	}
}
