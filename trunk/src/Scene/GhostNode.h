#ifndef _GHOSTNODE_H_
#define _GHOSTNODE_H_

#include "Common.h"
#include "Node.h"

/**
 * This is a node that does nothing
 */
class GhostNode: public Node
{
	public:
		GhostNode(): Node(NT_GHOST) { }
		void init( const char* ) {}
		void render() {}
		void deinit() {}
};


#endif
