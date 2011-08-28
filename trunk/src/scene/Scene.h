#ifndef SCENE_H
#define SCENE_H

#include <boost/scoped_ptr.hpp>
#include "phys/MasterContainer.h"
#include "util/Assert.h"
#include "util/Accessors.h"
#include "VisibilityTester.h"


class SceneNode;
class Light;
class Camera;
class Controller;
class ParticleEmitterNode;
class ModelNode;
class SkinNode;


/// The Scene contains all the dynamic entities
class Scene
{
	public:
		/// Typetraits
		template<typename Type>
		class Types
		{
			public:
				typedef Vec<Type*> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
		};

		enum
		{
			MAX_VISIBLE_NODES = 1000
		};

		Scene();
		~Scene();

		/// Put a node in the appropriate containers
		void registerNode(SceneNode* node);
		void unregisterNode(SceneNode* node);
		void registerController(Controller* controller);
		void unregisterController(Controller* controller);

		void updateAllWorldStuff(float prevUpdateTime, float crntTime);
		void updateAllControllers();
		void doVisibilityTests(Camera& cam) {visibilityTester->test(cam);}

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec3, ambientCol, getAmbientColor, setAmbientColor)
		phys::MasterContainer& getPhysMasterContainer();
		const phys::MasterContainer& getPhysMasterContainer() const;
		const VisibilityTester& getVisibilityTester() const;

		GETTER_RW(Types<SceneNode>::Container, nodes, getAllNodes)
		GETTER_RW(Types<Light>::Container, lights, getLights)
		GETTER_RW(Types<Camera>::Container, cameras, getCameras)
		GETTER_RW(Types<ParticleEmitterNode>::Container, particleEmitterNodes,
			getParticleEmitterNodes)
		GETTER_RW(Types<ModelNode>::Container, modelNodes, getModelNodes)
		GETTER_RW(Types<SkinNode>::Container, skinNodes, getSkinNodes)
		GETTER_RW(Types<Controller>::Container, controllers, getControllers)
		/// @}

	private:
		/// @name Containers of scene's data
		/// @{
		Types<SceneNode>::Container nodes;
		Types<Light>::Container lights;
		Types<Camera>::Container cameras;
		Types<ParticleEmitterNode>::Container particleEmitterNodes;
		Types<ModelNode>::Container modelNodes;
		Types<SkinNode>::Container skinNodes;
		Types<Controller>::Container controllers;
		/// @}

		Vec3 ambientCol; ///< The global ambient color
		/// Connection with Bullet wrapper
		boost::scoped_ptr<phys::MasterContainer> physMasterContainer;
		boost::scoped_ptr<VisibilityTester> visibilityTester;

		/// Adds a node in a container
		template<typename ContainerType, typename Type>
		void putBackNode(ContainerType& container, Type* x);

		/// Removes a node from a container
		template<typename ContainerType, typename Type>
		void eraseNode(ContainerType& container, Type* x);
};


template<typename ContainerType, typename Type>
inline void Scene::putBackNode(ContainerType& container, Type* x)
{
	ASSERT(std::find(container.begin(), container.end(), x) == container.end());
	container.push_back(x);
}


template<typename ContainerType, typename Type>
inline void Scene::eraseNode(ContainerType& container, Type* x)
{
	typename ContainerType::iterator it =
		std::find(container.begin(), container.end(), x);
	ASSERT(it != container.end());
	container.erase(it);
}


inline phys::MasterContainer& Scene::getPhysMasterContainer()
{
	return *physMasterContainer;
}


inline const phys::MasterContainer& Scene::getPhysMasterContainer() const
{
	return *physMasterContainer;
}


inline const VisibilityTester& Scene::getVisibilityTester() const
{
	return *visibilityTester;
}


#endif
