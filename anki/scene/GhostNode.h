#ifndef GHOST_NODE_H
#define GHOST_NODE_H

#include "anki/scene/SceneNode.h"

/**
 * This is a node that does nothing
 */
class GhostNode: public SceneNode
{
	public:
		GhostNode(Scene& scene): SceneNode(SNT_GHOST, scene, SNF_NONE, NULL) {}
		virtual ~GhostNode() {}
		void init(const char*) {}
};


#endif
