#ifndef SCENE_DRAWER_H
#define SCENE_DRAWER_H

#include <boost/variant.hpp>
#include "Math.h"
#include "MtlUserDefinedVar.h"


class RenderableNode;
class Renderer;
class Camera;
class Material;
class MaterialRuntime;
class MtlUserDefinedVarRuntime;


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
		/// @todo
		class UsrDefVarVisitor: public boost::static_visitor<void>
		{
			public:
				const MtlUserDefinedVarRuntime& udvr;
				const Renderer& r;
				mutable uint& texUnit;

				UsrDefVarVisitor(const MtlUserDefinedVarRuntime& udvr, const Renderer& r, uint& texUnit);

				void operator()(float x) const;
				void operator()(const Vec2& x) const;
				void operator()(const Vec3& x) const;
				void operator()(const Vec4& x) const;
				void operator()(const RsrcPtr<Texture>* x) const;
				void operator()(MtlUserDefinedVar::Fai x) const;
		};

		const Renderer& r; ///< Keep it here cause the class wants a few stuff from it

		/// This function:
		/// - binds the shader program
		/// - loads the uniforms
		/// - sets the GL state
		/// @param mtlr The material runtime
		/// @param nodeWorldTransform The world transformation to pass to the shader
		/// @param cam Needed for some matrices (view & projection)
		/// @param r The renderer, needed for some FAIs and some matrices
		static void setupShaderProg(const MaterialRuntime& mtlr, const Transform& nodeWorldTransform,
		                            const Camera& cam, const Renderer& r);
};


#endif
