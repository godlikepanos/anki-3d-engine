#ifndef _MESH_SKEL_CTRL_H_
#define _MESH_SKEL_CTRL_H_

#include "common.h"
#include "controller.h"


class mesh_node_t;
class skel_node_t;
class mesh_t;


/**
 * Skeleton controller
 * It controls a mesh node using a skeleton node and the skeleton node's controllers
 */
class mesh_skel_ctrl_t: public controller_t
{
	public:
		skel_node_t* skel_node;
		mesh_node_t* mesh_node;

		mesh_skel_ctrl_t( skel_node_t* skel_node_, mesh_node_t* mesh_node_ ):
			controller_t( CT_SKEL ),
			skel_node( skel_node_ ),
			mesh_node( mesh_node_ ) 
		{}
		/**
		 * Do nothing! We use HW skinning so its not necessary to update anything in the mesh_node. 
		 * The skel_node's controllers provide us with sufficient data to do the trick.
		 */
		void Update( float ) {}
};


#endif
