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
		/// Constructor
		VisibilityTester(Scene& scene);

		/// This method:
		/// - Gets the visible renderable nodes
		/// - Sort them from the closest to the farthest
		/// - Get the visible lights
		/// - For every spot light that casts shadow get the visible renderables
		void test(Camera& cam);

	//====================================================================================================================
	// Private                                                                                                           =
	//====================================================================================================================
	private:
		/// Used in sorting. Compare the length of 2 nodes from the camera
		struct CmpDistanceFromOrigin
		{
			Vec3 o; ///< The camera origin
			CmpDistanceFromOrigin(Vec3 o_): o(o_) {}
			bool operator()(const RenderableNode* a, const RenderableNode* b) const;
		};

		Scene& scene; ///< Know your father

		/// Test a node against the camera frustum
		template<typename Type>
		static bool test(const Type& tested, const Camera& cam);

		/// Get renderable nodes for a given camera
		/// @param skipShadowless Skip shadowless nodes. If the cam is a light cam
		/// @param cam The camera to test and gather renderable nodes
		void getRenderableNodes(bool skipShadowless, Camera& cam);
};


#endif
