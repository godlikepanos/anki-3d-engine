#ifndef ANKI_SCENE_VISIBILITY_TESTER_H
#define ANKI_SCENE_VISIBILITY_TESTER_H

#include "anki/math/Math.h"
#include "anki/core/ParallelJob.h"
#include <deque>
#include <boost/thread.hpp>


namespace anki {


class Camera;
class Scene;
class RenderableNode;
class SpotLight;
class PointLight;
class SceneNode;
class VisibilityInfo;


/// Performs visibility determination tests and fills a few containers with the
/// visible scene nodes
class VisibilityTester
{
	public:
		/// Constructor
		VisibilityTester(Scene& scene);
		~VisibilityTester();

		/// This method:
		/// - Gets the visible renderable nodes
		/// - Sort them from the closest to the farthest
		/// - Get the visible lights
		/// - For every spot light that casts shadow get the visible renderables
		void test(Camera& cam);

	private:
		/// Used in sorting. Compare the length of 2 nodes from the camera
		struct CmpDistanceFromOrigin
		{
			Vec3 o; ///< The camera origin
			CmpDistanceFromOrigin(const Vec3& o_): o(o_) {}
			bool operator()(const SceneNode* a, const SceneNode* b) const;
		};

		/// The JobParameters that we feed in the ParallelManager
		struct VisJobParameters: ParallelJobParameters
		{
			const Camera* cam;
			bool skipShadowless;
			VisibilityInfo* visibilityInfo;
			Scene* scene;
			boost::mutex* msRenderableNodesMtx;
			boost::mutex* bsRenderableNodesMtx;
		};

		Scene& scene; ///< Know your father

		/// @name Needed by getRenderableNodesJobCallback
		/// The vars of this group are needed by the static
		/// getRenderableNodesJobCallback and kept here so it can access them
		/// @{
		boost::mutex msRenderableNodesMtx;
		boost::mutex bsRenderableNodesMtx;
		/// @}

		/// Test a node against the camera frustum
		template<typename Type>
		static bool test(const Type& tested, const Camera& cam);

		/// Get renderable nodes for a given camera
		/// @param[in] skipShadowless Skip shadowless nodes. If the cam is a
		/// light cam
		/// @param[in] cam The camera to test and gather renderable nodes
		/// @param[out] storage The VisibilityInfo of where we will store the
		/// visible nodes
		void getRenderableNodes(bool skipShadowless, const Camera& cam,
			VisibilityInfo& storage);

		/// This static method will be fed into the ParallelManager
		/// @param data This is actually a pointer to VisibilityTester
		static void getRenderableNodesJobCallback(
			ParallelJobParameters& data,
			const ParallelJob& job);
};


} // end namespace


#endif
