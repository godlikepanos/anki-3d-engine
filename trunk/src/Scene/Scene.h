#ifndef _SCENE_H_
#define _SCENE_H_

#include "Common.h"
#include "skybox.h"
#include "PhyWorld.h"


class Node;
class Light;
class Camera;
class MeshNode;
class SkelNode;
class Controller;


/**
 * @brief The Scene contains all the dynamic entities
 */
class Scene
{
	PROPERTY_RW( Vec3, ambientCol, setAmbientCol, getAmbientCol ) ///< The global ambient color
	PROPERTY_RW( Vec3, sunPos, setSunPos, getSunPos )
	PROPERTY_RW( PhyWorld*, phyWorld, setPhyWorld, getPhyWorld ) ///< Connection with bullet

	private:
		/**
		 * @brief Adds a node in a container
		 */
		template<typename ContainerType, typename Type> void putBackNode( ContainerType& container, Type* x )
		{
			DEBUG_ERR( std::find( container.begin(), container.end(), x ) != container.end() );
			container.push_back( x );
		}

		/**
		 * @brief Removes a node from a container
		 */
		template<typename ContainerType, typename Type> void eraseNode( ContainerType& container, Type* x )
		{
			typename ContainerType::iterator it = std::find( container.begin(), container.end(), x );
			DEBUG_ERR( it == container.end() );
			container.erase( it );
		}

	public:
		/**
		 * @brief The container template class. Extends vector
		 */
		template<typename Type> class Container: public Vec<Type*>
		{};

		// Containers of scene's data
		Container<Node>       nodes;
		Container<Light>      lights;
		Container<Camera>     cameras;
		Container<MeshNode>   meshNodes;
		Container<SkelNode>   skelNodes;
		Container<Controller> controllers;
		Skybox                skybox; // ToDo to be removed

		// The funcs
		Scene();

		void registerNode( Node* node ); ///< Put a node in the appropriate containers
		void unregisterNode( Node* node );
		void registerController( Controller* controller );
		void unregisterController( Controller* controller );

		void updateAllWorldStuff();
		void updateAllControllers();
};

#endif
