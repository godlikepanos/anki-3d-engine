#ifndef _GHOSTNODE_H_
#define _GHOSTNODE_H_

#include "Common.h"
#include "SceneNode.h"

/**
 * This is a node that does nothing
 */
class GhostNode: public SceneNode
{
	public:
		GhostNode(): SceneNode(SNT_GHOST) {}
		~GhostNode() {}
		void init(const char*) {}
		void render() {}
};


#endif
