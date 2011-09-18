#ifndef SCENE_DRAWER_H
#define SCENE_DRAWER_H

#include "m/Math.h"
#include "rsrc/MaterialCommon.h"
#include "rsrc/RsrcPtr.h"
#include <boost/variant.hpp>


class RenderableNode;
class Camera;
class Material;
class MaterialRuntime;
class MaterialRuntimeVariable;


class Renderer;


/// It includes all the functions to render a RenderableNode
class SceneDrawer
{
	public:
		/// The one and only constructor
		SceneDrawer(const Renderer& r_)
		:	r(r_)
		{}

		void renderRenderableNode(const RenderableNode& renderable,
			const Camera& cam, PassType passType) const;

	private:
		/// Set the uniform using this visitor
		class UsrDefVarVisitor: public boost::static_visitor<>
		{
			public:
				const MaterialRuntimeVariable& udvr;
				const Renderer& r;
				PassType pt;
				uint& texUnit;

				UsrDefVarVisitor(const MaterialRuntimeVariable& udvr,
					const Renderer& r, PassType pt, uint& texUnit);

				/// Functor
				template<typename Type>
				void operator()(const Type& x) const;

				/// Functor
				void operator()(const RsrcPtr<Texture>* x) const;
		};

		const Renderer& r; ///< Keep it here cause the class wants a few stuff
		                   ///< from it

		/// This function:
		/// - binds the shader program
		/// - loads the uniforms
		/// - sets the GL state
		/// @param mtlr The material runtime
		/// @param nodeWorldTransform The world transformation to pass to the
		/// shader
		/// @param cam Needed for some matrices (view & projection)
		/// @param r The renderer, needed for some FAIs and some matrices
		static void setupShaderProg(
			const MaterialRuntime& mtlr,
			PassType pt,
			const Transform& nodeWorldTransform,
			const Camera& cam,
			const Renderer& r,
			float blurring);
};


#endif
