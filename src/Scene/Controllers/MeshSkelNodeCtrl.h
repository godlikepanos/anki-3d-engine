#ifndef _MESH_SKEL_CTRL_H_
#define _MESH_SKEL_CTRL_H_

#include "Common.h"
#include "Controller.h"
#include "Vbo.h"


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

		struct
		{
			Vbo positions;
			Vbo normals;
			Vbo tangents;
		} vbos;


		MeshSkelNodeCtrl( SkelNode* skelNode_, MeshNode* meshNode_ ):
			Controller( CT_SKEL ),
			skelNode( skelNode_ ),
			meshNode( meshNode_ )
		{}

		/**
		 * Do nothing! We use HW skinning so its not necessary to update anything in the meshNode. 
		 * The skelNode's controllers provide us with sufficient data to do the trick.
		 */
		void update( float ) {}
};


#endif
