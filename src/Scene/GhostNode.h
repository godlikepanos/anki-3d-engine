#ifndef GHOST_NODE_H
#define GHOST_NODE_H

#include "SceneNode.h"

/**
 * This is a node that does nothing
 */
class GhostNode: public SceneNode
{
	public:
		GhostNode(): SceneNode(SNT_GHOST, false, NULL) {}
		~GhostNode() {}
		void init(const char*) {}
};


#endif
