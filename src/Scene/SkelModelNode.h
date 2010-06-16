#ifndef _SKEL_MODEL_NODE_H_
#define _SKEL_MODEL_NODE_H_

#include "Common.h"
#include "MeshNode.h" 


class MeshNode;
class SkelNode;


/** 
 * Skeleton model Scene node
 * It is just a group node with a derived init
 */
class SkelModelNode: public SceneNode
{
	public:
		Vec<MeshNode*> meshNodes;
		SkelNode*   skelNode;
		
		SkelModelNode(): SceneNode(NT_SKEL_MODEL), skelNode(NULL) { isCompound = true; }
		void init(const char* filename);
		void deinit() {} ///< Do nothing because it loads no resources
		void render() {} ///< Do nothing
};

#endif
