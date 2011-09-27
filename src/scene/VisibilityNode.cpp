#include "VisibilityNode.h"


//==============================================================================
VisibilityNode::~VisibilityNode()
{}


//==============================================================================
void VisibilityNode::clearAll()
{
	msRNodes.clear();
	bsRNodes.clear();
	pLights.clear();
	sLights.clear();
}
