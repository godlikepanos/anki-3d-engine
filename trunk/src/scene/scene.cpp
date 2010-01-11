#include "scene.h"
#include "skel_node.h"

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
// RegisterNode                                                                                                                       =
//=====================================================================================================================================
template<container_type_t, type_t> static void RegisterNode( container_type_t& container, type_t* x )
{
	DEBUG_ERR( std::find( container.begin(), container.end(), x ) != container.end() );
	container.push_back( x );
}

template<container_type_t, type_t> static void UbregisterNode( container_type_t& container, type_t* x )
{
	container::iterator it = std::find( container.begin(), container.end(), x );
	DEBUG_ERR( it == container.end() );
	container.erase( it );
}


//=====================================================================================================================================
// RegisterNodeAndChilds                                                                                                              =
//=====================================================================================================================================
void RegisterNodeAndChilds( node_t* node )
{
	RegisterNode( nodes, node );
	
	switch( node->type )
	{
		case node_t::NT_LIGHT:
			RegisterNode( lights, static_cast<light_t*>(node) );
			break;
		case node_t::NT_CAMERA:
			RegisterNode( cameras, static_cast<light_t*>(camera) );
			break;
		case node_t::NT_MESH:
			RegisterNode( mesh_nodes, static_cast<mesh_node_t*>(node) );
			break;
		case node_t::NT_SKELETON:
			RegisterNode( skel_nodes, static_cast<skel_node_t*>(node) );
			break;
		case node_t::NT_SKEL_MODEL:
			// ToDo
			break;
	};
	
	// now register the childs
	for( vec_t<node_t*>::iterator it=node->childs.begin(); it!=node->childs.end(); it++ )
		RegisterNodeAndChilds( it );
}


//=====================================================================================================================================
// UnregisterNodeAndChilds                                                                                                            =
//=====================================================================================================================================
void UnregisterNodeAndChilds( node_t* node )
{
	UnregisterNode( nodes, node );
	
	switch( node->type )
	{
		case node_t::NT_LIGHT:
			UnregisterNode( lights, static_cast<light_t*>(node) );
			break;
		case node_t::NT_CAMERA:
			UnregisterNode( cameras, static_cast<light_t*>(camera) );
			break;
		case node_t::NT_MESH:
			UnregisterNode( mesh_nodes, static_cast<mesh_node_t*>(node) );
			break;
		case node_t::NT_SKELETON:
			UnregisterNode( skel_nodes, static_cast<skel_node_t*>(node) );
			break;
		case node_t::NT_SKEL_MODEL:
			// ToDo
			break;
	};
	
	// now register the childs
	for( vec_t<node_t*>::iterator it=node->childs.begin(); it!=node->childs.end(); it++ )
		UnregisterNodeAndChilds( it );
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


//=====================================================================================================================================
// Nodet                                                                                                                              =
//=====================================================================================================================================
void container_node_t::Register( node_t* x )
{
	RegisterMe( x );
}

void container_node_t::Unregister( node_t* x )
{
	UnregisterMe( x );
}


//=====================================================================================================================================
// Camera                                                                                                                             =
//=====================================================================================================================================
void container_camera_t::Register( camera_t* x )
{
	RegisterMe( x );
	nodes.Register( x );
}

void container_camera_t::Unregister( camera_t* x )
{
	UnregisterMe( x );
	nodes.Unregister( x );
}


//=====================================================================================================================================
// Light                                                                                                                              =
//=====================================================================================================================================
void container_light_t::Register( light_t* x )
{
	RegisterMe( x );
	nodes.Register( x );

	if( x->GetType() == light_t::LT_SPOT )
	{
		spot_light_t* spot = static_cast<spot_light_t*>(x);
		cameras.Register( &spot->camera );
	}
}

void container_light_t::Unregister( light_t* x )
{
	UnregisterMe( x );
	nodes.Unregister( x );

	if( x->GetType() == light_t::LT_SPOT )
	{
		spot_light_t* spot = static_cast<spot_light_t*>(x);
		cameras.Unregister( &spot->camera );
	}
}


//=====================================================================================================================================
// Mesh node                                                                                                                          =
//=====================================================================================================================================
void container_mesh_node_t::Register( mesh_node_t* x )
{
	RegisterMe( x );
	nodes.Register( x );
}

void container_mesh_node_t::Unregister( mesh_node_t* x )
{
	UnregisterMe( x );
	nodes.Unregister( x );
}


} // end namespace
