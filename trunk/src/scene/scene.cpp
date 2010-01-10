#include "scene.h"
#include "skel_node.h"

namespace scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
skybox_t skybox;

extern container_node_t       nodes;
extern container_light_t      lights;
extern container_camera_t     cameras;
extern container_mesh_node_t  mesh_nodes;
extern container_skel_node_t  skel_nodes;


/*
=======================================================================================================================================
UpdateAllWorldStuff                                                                                                                   =
=======================================================================================================================================
*/
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
