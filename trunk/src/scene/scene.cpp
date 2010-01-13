#include <algorithm>
#include "scene.h"
#include "skel_node.h"
#include "camera.h"

namespace scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
skybox_t skybox;

container_node_t       nodes;
container_light_t      lights;
container_camera_t     cameras;
container_mesh_node_t  mesh_nodes;
container_skel_node_t  skel_nodes;


//=====================================================================================================================================
// Static template funcs                                                                                                              =
//=====================================================================================================================================
template<typename container_type_t, typename type_t> static void PutBackNode( container_type_t& container, type_t* x )
{
	DEBUG_ERR( std::find( container.begin(), container.end(), x ) != container.end() );
	container.push_back( x );
}

template<typename container_type_t, typename type_t> static void EraseNode( container_type_t& container, type_t* x )
{
	typename container_type_t::iterator it = std::find( container.begin(), container.end(), x );
	DEBUG_ERR( it == container.end() );
	container.erase( it );
}


//=====================================================================================================================================
// RegisterNode                                                                                                                       =
//=====================================================================================================================================
void RegisterNode( node_t* node )
{
	PutBackNode( nodes, node );
	
	switch( node->type )
	{
		case node_t::NT_LIGHT:
			PutBackNode( lights, static_cast<light_t*>(node) );
			break;
		case node_t::NT_CAMERA:
			PutBackNode( cameras, static_cast<camera_t*>(node) );
			break;
		case node_t::NT_MESH:
			PutBackNode( mesh_nodes, static_cast<mesh_node_t*>(node) );
			break;
		case node_t::NT_SKELETON:
			PutBackNode( skel_nodes, static_cast<skel_node_t*>(node) );
			break;
		case node_t::NT_SKEL_MODEL:
			// ToDo
			break;
	};
}


//=====================================================================================================================================
// UnregisterNode                                                                                                                     =
//=====================================================================================================================================
void UnregisterNode( node_t* node )
{
	EraseNode( nodes, node );
	
	switch( node->type )
	{
		case node_t::NT_LIGHT:
			EraseNode( lights, static_cast<light_t*>(node) );
			break;
		case node_t::NT_CAMERA:
			EraseNode( cameras, static_cast<camera_t*>(node) );
			break;
		case node_t::NT_MESH:
			EraseNode( mesh_nodes, static_cast<mesh_node_t*>(node) );
			break;
		case node_t::NT_SKELETON:
			EraseNode( skel_nodes, static_cast<skel_node_t*>(node) );
			break;
		case node_t::NT_SKEL_MODEL:
			// ToDo
			break;
	};
}


//=====================================================================================================================================
// UpdateAllWorldStuff                                                                                                                =
//=====================================================================================================================================
void UpdateAllWorldStuff()
{
	DEBUG_ERR( nodes.size() > 1024 );
	node_t* queue [1024];
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	for( uint i=0; i<nodes.size(); i++ )
		if( nodes[i]->parent == NULL )
			queue[tail++] = nodes[i]; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		node_t* obj = queue[head++]; // queue pop

		obj->UpdateWorldStuff();
		++num;

		for( uint i=0; i<obj->childs.size(); i++ )
			queue[tail++] = obj->childs[i];
	}

	DEBUG_ERR( num != nodes.size() );
}


//=====================================================================================================================================
// UpdateAllSkeletonNodes                                                                                                             =
//=====================================================================================================================================
void UpdateAllSkeletonNodes()
{
	for( uint i=0; i<skel_nodes.size(); i++ )
	{
		skel_nodes[i]->skel_anim_controller->Update();
	}
}


} // end namespace
