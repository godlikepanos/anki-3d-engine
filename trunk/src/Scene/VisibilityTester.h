#ifndef VISIBILITY_TESTER_H
#define VISIBILITY_TESTER_H

#include <deque>
#include "Properties.h"
#include "Math.h"


class Camera;
class Scene;
class SceneRenderable;
class SpotLight;
class PointLight;


/// Performs visibility determination tests and fills a few containers with the visible scene nodes
class VisibilityTester
{
	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/// Types
		template<typename Type>
		class Types
		{
			public:
				typedef std::deque<Type> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
		};

		/// The information about the visible lights
		template<typename LightType>
		class VisibleLight
		{
			friend class VisibilityTester;

			public:
				const LightType& getLight() const {return *light;}
				GETTER_R(Types<const SceneRenderable*>::Container, renderables, getRenderables)

			private:
				const LightType* light;
				Types<const SceneRenderable*>::Container renderables; ///< The visible nodes by that light
		};

		/// Constructor
		VisibilityTester(const Scene& scene);

		/// @name Accessors
		/// @{
		GETTER_R(Types<const SceneRenderable*>::Container, msRenderables, getMsRenderables)
		GETTER_R(Types<const SceneRenderable*>::Container, bsRenderables, getBsRenderables)
		GETTER_R(Types<VisibleLight<PointLight> >::Container, pointLights, getPointLights)
		GETTER_R(Types<VisibleLight<SpotLight> >::Container, spotLights, getSpotLights)
		/// @}

		void test(const Camera& cam);

	//====================================================================================================================
	// Private                                                                                                           =
	//====================================================================================================================
	private:
		/// Used in sorting. Compare the length of 2 nodes from the camera
		struct CmpLength
		{
			Vec3 o; ///< The camera origin
			CmpLength(Vec3 o_): o(o_) {}
			bool operator()(const SceneRenderable* a, const SceneRenderable* b) const;
		};

		const Scene& scene; ///< Know your father

		Types<const SceneRenderable*>::Container msRenderables;
		Types<const SceneRenderable*>::Container bsRenderables;
		Types<VisibleLight<PointLight> >::Container pointLights;
		Types<VisibleLight<SpotLight> >::Container spotLights;

		/// @todo write some real code
		static bool test(const SceneRenderable& /*renderable*/, const Camera& /*cam*/) {return true;}
};


#endif
