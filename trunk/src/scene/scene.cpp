#include <algorithm>
#include "scene.h"
#include "skel_node.h"
#include "camera.h"
#include "mesh_node.h"
#include "light.h"
#include "controller.h"

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

vec_t<controller_t*>   controllers;


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
// Register and Unregister controllers                                                                                                =
//=====================================================================================================================================
void RegisterController( controller_t* controller )
{
	DEBUG_ERR( std::find( controllers.begin(), controllers.end(), controller ) != controllers.end() );
	controllers.push_back( controller );
}

void UnregisterController( controller_t* controller )
{
	vec_t<controller_t*>::iterator it = std::find( controllers.begin(), controllers.end(), controller );
	DEBUG_ERR( it == controllers.end() );
	controllers.erase( it );
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
// UpdateAllControllers                                                                                                               =
//=====================================================================================================================================
void UpdateAllControllers()
{
	/*for( container_node_t::iterator it=nodes.begin(); it!=nodes.end(); it++ )
	{
		node_t* node = (*it);
		for( vec_t<controller_t*>::iterator it1=node->controllers.begin(); it1!=node->controllers.end(); it1++ )
			(*it1)->Update( 0.0 );
	}*/
	
	for( vec_t<controller_t*>::iterator it=controllers.begin(); it!=controllers.end(); it++ )
		(*it)->Update( 0.0 );
}


} // end namespace
