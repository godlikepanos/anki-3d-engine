#include <algorithm>
#include "Scene.h"
#include "SkelNode.h"
#include "Camera.h"
#include "MeshNode.h"
#include "Light.h"
#include "Controller.h"
#include "Material.h"

namespace Scene {

/*
=======================================================================================================================================
DATA                                                                                                                                  =
=======================================================================================================================================
*/
Skybox skybox;

NodeContainer     nodes;
LightContainer    lights;
CameraContainer   cameras;
MeshContainer     meshNodes;
SkelNodeContainer skelNodes;

Vec<Controller*> controllers;


//=====================================================================================================================================
// Static template funcs                                                                                                              =
//=====================================================================================================================================
template<typename ContainerType, typename Type> static void PutBackNode( ContainerType& container, Type* x )
{
	DEBUG_ERR( std::find( container.begin(), container.end(), x ) != container.end() );
	container.push_back( x );
}

template<typename ContainerType, typename Type> static void EraseNode( ContainerType& container, Type* x )
{
	typename ContainerType::iterator it = std::find( container.begin(), container.end(), x );
	DEBUG_ERR( it == container.end() );
	container.erase( it );
}


//=====================================================================================================================================
// registerNode                                                                                                                       =
//=====================================================================================================================================
void registerNode( Node* node )
{
	PutBackNode( nodes, node );
	
	switch( node->type )
	{
		case Node::NT_LIGHT:
			PutBackNode( lights, static_cast<Light*>(node) );
			break;
		case Node::NT_CAMERA:
			PutBackNode( cameras, static_cast<Camera*>(node) );
			break;
		case Node::NT_MESH:
			PutBackNode( meshNodes, static_cast<MeshNode*>(node) );
			break;
		case Node::NT_SKELETON:
			PutBackNode( skelNodes, static_cast<SkelNode*>(node) );
			break;
		case Node::NT_SKEL_MODEL:
			// ToDo
			break;
	};
}


//=====================================================================================================================================
// unregisterNode                                                                                                                     =
//=====================================================================================================================================
void unregisterNode( Node* node )
{
	EraseNode( nodes, node );
	
	switch( node->type )
	{
		case Node::NT_LIGHT:
			EraseNode( lights, static_cast<Light*>(node) );
			break;
		case Node::NT_CAMERA:
			EraseNode( cameras, static_cast<Camera*>(node) );
			break;
		case Node::NT_MESH:
			EraseNode( meshNodes, static_cast<MeshNode*>(node) );
			break;
		case Node::NT_SKELETON:
			EraseNode( skelNodes, static_cast<SkelNode*>(node) );
			break;
		case Node::NT_SKEL_MODEL:
			// ToDo
			break;
	};
}


//=====================================================================================================================================
// Register and Unregister controllers                                                                                                =
//=====================================================================================================================================
void registerController( Controller* controller )
{
	DEBUG_ERR( std::find( controllers.begin(), controllers.end(), controller ) != controllers.end() );
	controllers.push_back( controller );
}

void unregisterController( Controller* controller )
{
	Vec<Controller*>::iterator it = std::find( controllers.begin(), controllers.end(), controller );
	DEBUG_ERR( it == controllers.end() );
	controllers.erase( it );
}


//=====================================================================================================================================
// updateAllWorldStuff                                                                                                                =
//=====================================================================================================================================
void updateAllWorldStuff()
{
	DEBUG_ERR( nodes.size() > 1024 );
	Node* queue [1024];
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	for( uint i=0; i<nodes.size(); i++ )
		if( nodes[i]->parent == NULL )
			queue[tail++] = nodes[i]; // queue push

	// loop
	while( head != tail ) // while queue not empty
	{
		Node* obj = queue[head++]; // queue pop

		obj->updateWorldStuff();
		++num;

		for( uint i=0; i<obj->childs.size(); i++ )
			queue[tail++] = obj->childs[i];
	}

	DEBUG_ERR( num != nodes.size() );
}


//=====================================================================================================================================
// updateAllControllers                                                                                                               =
//=====================================================================================================================================
void updateAllControllers()
{
	/*for( NodeContainer::iterator it=nodes.begin(); it!=nodes.end(); it++ )
	{
		Node* node = (*it);
		for( Vec<Controller*>::iterator it1=node->controllers.begin(); it1!=node->controllers.end(); it1++ )
			(*it1)->update( 0.0 );
	}*/

	for( Vec<Controller*>::iterator it=controllers.begin(); it!=controllers.end(); it++ )
	{
		(*it)->update( 0.0 );
	}
}


} // end namespace
