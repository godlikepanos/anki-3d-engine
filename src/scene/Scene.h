#ifndef SCENE_H
#define SCENE_H

#include <boost/scoped_ptr.hpp>
#include "phys/PhysWorld.h"
#include "util/Assert.h"
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
			MAX_VISIBLE_NODES = 1024
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
		const Vec3& getAmbientColor() const {return ambientCol;}
		Vec3& getAmbientColor() {return ambientCol;}
		void setAmbientColor(const Vec3& x) {ambientCol = x;}

		PhysWorld& getPhysPhysWorld();
		const PhysWorld& getPhysPhysWorld() const;

		const VisibilityTester& getVisibilityTester() const;

		const Types<SceneNode>::Container& getAllNodes() const {return nodes;}
		Types<SceneNode>::Container& getAllNodes() {return nodes;}

		const Types<Light>::Container& getLights() const {return lights;}
		Types<Light>::Container& getLights() {return lights;}

		const Types<Camera>::Container& getCameras() const {return cameras;}
		Types<Camera>::Container& getCameras() {return cameras;}

		const Types<ParticleEmitterNode>::Container& getParticleEmitterNodes()
			const {return particleEmitterNodes;}
		Types<ParticleEmitterNode>::Container& getParticleEmitterNodes()
			{return particleEmitterNodes;}

		const Types<ModelNode>::Container& getModelNodes() const
			{return modelNodes;}
		Types<ModelNode>::Container& getModelNodes() {return modelNodes;}

		const Types<SkinNode>::Container& getSkinNodes() const
			{return skinNodes;}
		Types<SkinNode>::Container& getSkinNodes() {return skinNodes;}

		const Types<Controller>::Container& getControllers() const
			{return controllers;}
		Types<Controller>::Container& getControllers() {return controllers;}
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
		boost::scoped_ptr<PhysWorld> physPhysWorld;
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


inline PhysWorld& Scene::getPhysPhysWorld()
{
	return *physPhysWorld;
}


inline const PhysWorld& Scene::getPhysPhysWorld() const
{
	return *physPhysWorld;
}


inline const VisibilityTester& Scene::getVisibilityTester() const
{
	return *visibilityTester;
}


#endif
