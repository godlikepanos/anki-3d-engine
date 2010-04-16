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
		GhostNode(): SceneNode(NT_GHOST) { }
		void init( const char* ) {}
		void render() {}
		void deinit() {}
};


#endif
