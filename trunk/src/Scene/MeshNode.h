#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "Common.h"
#include "Node.h"
#include "Material.h"

class MeshSkelNodeCtrl;
class Mesh;


/// Mesh node
class MeshNode: public Node
{
	private:
		void render( Material* mtl ) const; ///< Common code for render() and renderDepth()

	public:
		// resources
		Mesh* mesh;
		Material* material;
		Material* dpMaterial; ///< Depth pass material
		// controllers
		MeshSkelNodeCtrl* meshSkelCtrl;
		// funcs
		MeshNode(): Node(NT_MESH), meshSkelCtrl(NULL) {}
		virtual void render() { render(material); }
		virtual void renderDepth() { render( material->dpMtl ); }
		void init( const char* filename );
		void deinit();
};


#endif
