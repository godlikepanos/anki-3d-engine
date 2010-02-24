#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "common.h"
#include "node.h"
#include "Material.h"

class mesh_skel_ctrl_t;
class Mesh;


/// Mesh node
class mesh_node_t: public node_t
{
	private:
		void Render( Material* mtl ) const; ///< Common code for Render() and RenderDepth()

	public:
		// resources
		Mesh* mesh;
		Material* material;
		Material* dp_material; ///< Depth pass material
		// controllers
		mesh_skel_ctrl_t* mesh_skel_ctrl;
		// funcs
		mesh_node_t(): node_t(NT_MESH), mesh_skel_ctrl(NULL) {}
		virtual void Render() { Render(material); }
		virtual void RenderDepth() { Render( material->dp_mtl ); }
		void Init( const char* filename );
		void Deinit();
};


#endif
