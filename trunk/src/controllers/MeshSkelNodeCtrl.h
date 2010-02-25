#ifndef _MESH_SKEL_CTRL_H_
#define _MESH_SKEL_CTRL_H_

#include "Common.h"
#include "Controller.h"


class MeshNode;
class SkelNode;
class Mesh;


/**
 * Skeleton controller
 * It controls a mesh node using a skeleton node and the skeleton node's controllers
 */
class MeshSkelNodeCtrl: public Controller
{
	public:
		SkelNode* skelNode;
		MeshNode* meshNode;

		MeshSkelNodeCtrl( SkelNode* skel_node_, MeshNode* mesh_node_ ):
			Controller( CT_SKEL ),
			skelNode( skel_node_ ),
			meshNode( mesh_node_ ) 
		{}
		/**
		 * Do nothing! We use HW skinning so its not necessary to update anything in the meshNode. 
		 * The skelNode's controllers provide us with sufficient data to do the trick.
		 */
		void update( float ) {}
};


#endif
