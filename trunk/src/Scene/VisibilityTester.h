#ifndef VISIBILITY_TESTER_H
#define VISIBILITY_TESTER_H

#include <deque>
#include "Properties.h"
#include "Math.h"


class Camera;
class Scene;
class RenderableNode;
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
				GETTER_R(Types<const RenderableNode*>::Container, renderables, getRenderables)

			private:
				const LightType* light;
				Types<const RenderableNode*>::Container renderables; ///< The visible nodes by that light
		};

		/// Constructor
		VisibilityTester(const Scene& scene);

		/// @name Accessors
		/// @{
		GETTER_R(Types<const RenderableNode*>::Container, msRenderables, getMsRenderables)
		GETTER_R(Types<const RenderableNode*>::Container, bsRenderables, getBsRenderables)
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
			bool operator()(const RenderableNode* a, const RenderableNode* b) const;
		};

		const Scene& scene; ///< Know your father

		Types<const RenderableNode*>::Container msRenderables;
		Types<const RenderableNode*>::Container bsRenderables;
		Types<VisibleLight<PointLight> >::Container pointLights;
		Types<VisibleLight<SpotLight> >::Container spotLights;

		/// @todo write some real code
		static bool test(const RenderableNode& /*renderable*/, const Camera& /*cam*/) {return true;}
};


#endif
