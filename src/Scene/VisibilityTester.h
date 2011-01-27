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
	public:
		/// Types
		template<typename Type>
		class Types
		{
			public:
				typedef std::deque<const Type*> Container;
				typedef typename Container::iterator Iterator;
				typedef typename Container::const_iterator ConstIterator;
		};

		/// Constructor
		VisibilityTester(const Scene& scene);

		/// @name Accessors
		/// @{
		GETTER_R(Types<SceneRenderable>::Container, msRenderables, getMsRenderables)
		GETTER_R(Types<SceneRenderable>::Container, bsRenderables, getBsRenderables)
		GETTER_R(Types<PointLight>::Container, pointLights, getPointLights)
		GETTER_R(Types<SpotLight>::Container, spotLights, getSpotLights)
		/// @}

		void test(const Camera& cam);

	private:
		/// Used in sorting. Compare the length of 2 nodes from the camera
		struct CmpLength
		{
			Vec3 o; ///< The camera origin
			CmpLength(Vec3 o_): o(o_) {}
			bool operator()(const SceneRenderable* a, const SceneRenderable* b) const;
		};

		const Scene& scene; ///< Know your father

		Types<SceneRenderable>::Container msRenderables;
		Types<SceneRenderable>::Container bsRenderables;
		Types<PointLight>::Container pointLights;
		Types<SpotLight>::Container spotLights;
};


#endif
