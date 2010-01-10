#ifndef _MESH_NODE_H_
#define _MESH_NODE_H_

#include "common.h"
#include "node.h"
#include "controller.h"


class mesh_node_t;
class skel_node_t;
class mesh_t;


/// Skeleton controller
class skel_controller_t: public controller_t<mesh_node_t>
{
	public:
		skel_node_t* skel_node;
		mesh_node_t* mesh_node;

		skel_controller_t( mesh_node_t* mesh_node_ ): mesh_node(mesh_node_) {}
		void Update() {}
};


/// Mesh node
class mesh_node_t: public node_t
{
	public:
		mesh_t* mesh;
		material_t* material;
		skel_controller_t* skel_controller;

		mesh_node_t(): node_t(NT_MESH) {}

		void Render();
		void RenderDepth();
		void Init( const char* filename );
		void Deinit();
};


#endif
