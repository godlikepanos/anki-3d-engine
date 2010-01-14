#ifndef _SKEL_MODEL_NODE_H_
#define _SKEL_MODEL_NODE_H_

#include "common.h"
#include "mesh_node.h" 


class mesh_node_t;
class skel_node_t;


/** 
 * Skeleton model scene node
 * It is just a group node with a derived Init
 */
class skel_model_node_t: public node_t
{
	public:
		vec_t<mesh_node_t*> mesh_nodes;
		skel_node_t* skel_node;
		
		skel_model_node_t(): node_t(NT_SKEL_MODEL), skel_node(NULL) { is_group_node = true; }
		void Init( const char* filename );
		void Deinit() {} ///< Do nothing because it loads no resources
		void Render() {} ///< Do nothing
};

#endif
