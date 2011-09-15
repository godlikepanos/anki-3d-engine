#ifndef SCENE_H
#define SCENE_H

#include "phys/PhysWorld.h"
#include "util/Assert.h"
#include "VisibilityTester.h"
#include <boost/scoped_ptr.hpp>
#include <boost/range/iterator_range.hpp>


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
		template<typename T>
		class Types
		{
			public:
				typedef Vec<T*> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
				typedef boost::iterator_range<Iterator> MutableRange;
				typedef boost::iterator_range<ConstIterator> ConstRange;
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

		Types<SceneNode>::ConstRange getAllNodes() const;
		Types<SceneNode>::MutableRange getAllNodes();

		Types<Light>::ConstRange getLights() const;
		Types<Light>::MutableRange getLights();

		Types<Camera>::ConstRange getCameras() const;
		Types<Camera>::MutableRange getCameras();

		Types<ParticleEmitterNode>::ConstRange getParticleEmitterNodes() const;
		Types<ParticleEmitterNode>::MutableRange getParticleEmitterNodes();

		Types<ModelNode>::ConstRange getModelNodes() const;
		Types<ModelNode>::MutableRange getModelNodes();

		Types<SkinNode>::ConstRange getSkinNodes() const;
		Types<SkinNode>::MutableRange getSkinNodes();

		const Types<Controller>::Container& getControllers() const
			{return controllers;}
		Types<Controller>::Container& getControllers() {return controllers;}
		/// @}

	private:
		/// @name Containers of scene's data
		/// @{
		Types<SceneNode>::Container nodes;
		Types<Light>::Container lights;
		Types<Camera>::Container cams;
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


#include "Scene.inl.h"


#endif
