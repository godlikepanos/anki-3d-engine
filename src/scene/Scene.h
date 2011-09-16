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

		void doVisibilityTests(Camera& cam)
		{
			visibilityTester->test(cam);
		}

		/// @name Accessors
		/// @{
		const Vec3& getAmbientColor() const
		{
			return ambientCol;
		}
		Vec3& getAmbientColor()
		{
			return ambientCol;
		}
		void setAmbientColor(const Vec3& x)
		{
			ambientCol = x;
		}

		PhysWorld& getPhysWorld()
		{
			return *physPhysWorld;
		}
		const PhysWorld& getPhysWorld() const
		{
			return *physPhysWorld;
		}

		const VisibilityTester& getVisibilityTester() const
		{
			return *visibilityTester;
		}

		Types<SceneNode>::ConstRange getAllNodes() const
		{
			return Types<SceneNode>::ConstRange(nodes.begin(), nodes.end());
		}
		Types<SceneNode>::MutableRange getAllNodes()
		{
			return Types<SceneNode>::MutableRange(nodes.begin(), nodes.end());
		}

		Types<Light>::ConstRange getLights() const
		{
			return Types<Light>::ConstRange(lights.begin(), lights.end());
		}
		Types<Light>::MutableRange getLights()
		{
			return Types<Light>::MutableRange(lights.begin(), lights.end());
		}

		Types<Camera>::ConstRange getCameras() const
		{
			return Types<Camera>::ConstRange(cams.begin(), cams.end());
		}
		Types<Camera>::MutableRange getCameras()
		{
			return Types<Camera>::MutableRange(cams.begin(), cams.end());
		}

		Types<ParticleEmitterNode>::ConstRange getParticleEmitterNodes() const
		{
			return Types<ParticleEmitterNode>::ConstRange(
				particleEmitterNodes.begin(),
				particleEmitterNodes.end());
		}
		Types<ParticleEmitterNode>::MutableRange getParticleEmitterNodes()
		{
			return Types<ParticleEmitterNode>::MutableRange(
				particleEmitterNodes.begin(),
				particleEmitterNodes.end());
		}


		Types<ModelNode>::ConstRange getModelNodes() const
		{
			return Types<ModelNode>::ConstRange(modelNodes.begin(),
				modelNodes.end());
		}
		Types<ModelNode>::MutableRange getModelNodes()
		{
			return Types<ModelNode>::MutableRange(modelNodes.begin(),
				modelNodes.end());
		}

		Types<SkinNode>::ConstRange getSkinNodes() const
		{
			return Types<SkinNode>::ConstRange(skinNodes.begin(),
				skinNodes.end());
		}
		Types<SkinNode>::MutableRange getSkinNodes()
		{
			return Types<SkinNode>::MutableRange(skinNodes.begin(),
				skinNodes.end());
		}

		const Types<Controller>::Container& getControllers() const
		{
			return controllers;
		}
		Types<Controller>::Container& getControllers()
		{
			return controllers;
		}
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
