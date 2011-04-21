#include "GlStateMachine.h"


//======================================================================================================================
// sync                                                                                                                =
//======================================================================================================================
void GlStateMachine::sync()
{
	depthTestEnabledFlag = glIsEnabled(GL_DEPTH_TEST);
	blendingEnabledFlag = glIsEnabled(GL_BLEND);
	sProgGlId = getCurrentProgramGlId();
}
