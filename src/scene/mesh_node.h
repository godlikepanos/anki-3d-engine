#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "common.h"
#include "node.h"


class skel_controller_t;
class mesh_t;
class material_t;


/// Mesh node
class mesh_node_t: public node_t
{
	private:
		void Render( material_t* mtl ) const; ///< Common code for Render() and RenderDepth()

	public:
		// resources
		mesh_t* mesh;
		material_t* material;
		material_t* dp_material; ///< Depth pass material
		// controllers
		skel_controller_t* skel_controller;
		// funcs
		mesh_node_t(): node_t(NT_MESH), skel_controller(NULL) {}
		void Render() { Render(material); }
		void RenderDepth() { Render( material->dp_mtl ); }
		void Init( const char* filename );
		void Deinit();
};


#endif
