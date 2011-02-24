#ifndef SCENE_DRAWER_H
#define SCENE_DRAWER_H

#include "Math.h"


class RenderableNode;
class Renderer;
class Camera;
class Material;


/// It includes all the functions to render a RenderableNode
class SceneDrawer
{
	public:
		enum RenderingPassType
		{
			RPT_COLOR,
			RPT_DEPTH
		};

		/// The one and only contructor
		SceneDrawer(const Renderer& r_): r(r_) {}

		void renderRenderableNode(const RenderableNode& renderable, const Camera& cam, RenderingPassType rtype) const;

	private:
		const Renderer& r; ///< Keep it here cause the class wants a few stuff from it

		/// This function:
		/// - binds the shader program
		/// - loads the uniforms
		/// - sets the GL state
		/// @param mtl The material containing the shader program and the locations
		/// @param nodeWorldTransform The world transformation to pass to the shader
		/// @param cam Needed for some matrices (view & projection)
		/// @param r The renderer, needed for some FAIs and some matrices
		static void setupShaderProg(const Material& mtl, const Transform& nodeWorldTransform, const Camera& cam,
		                            const Renderer& r);
};


#endif
