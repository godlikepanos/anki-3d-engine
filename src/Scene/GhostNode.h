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
		virtual ~GhostNode() {}
		void init(const char*) {}

		void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) {}
};


#endif
