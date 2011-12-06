#ifndef ANKI_RENDERER_SCENE_DRAWER_H
#define ANKI_RENDERER_SCENE_DRAWER_H

#include "anki/math/Math.h"
#include "anki/resource/MaterialCommon.h"
#include "anki/resource/Resource.h"
#include <boost/variant.hpp>


namespace anki {


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
		: r(r_)
	{}

	void renderRenderableNode(const RenderableNode& renderable,
		const Camera& cam, const PassLevelKey& key) const;

private:
	/// Standard attribute variables that are acceptable inside the
	/// ShaderProgram
	enum ShaderProgramVariable_
	{
			// Attributes
			SPV_POSITION,
			SPV_TANGENT,
			SPV_NORMAL,
			SPV_TEX_COORDS,
			// Uniforms
			// Matrices
			SPV_MODEL_MAT,
			SPV_VIEW_MAT,
			SPV_PROJECTION_MAT,
			SPV_MODELVIEW_MAT,
			SPV_VIEWPROJECTION_MAT,
			SPV_NORMAL_MAT,
			SPV_MODELVIEWPROJECTION_MAT,
			// FAIs (for materials in blending stage)
			SPV_MS_NORMAL_FAI,
			SPV_MS_DIFFUSE_FAI,
			SPV_MS_SPECULAR_FAI,
			SPV_MS_DEPTH_FAI,
			SPV_IS_FAI,
			SPV_PPS_PRE_PASS_FAI,
			SPV_PPS_POST_PASS_FAI,
			// Other
			SPV_RENDERER_SIZE,
			SPV_SCENE_AMBIENT_COLOR,
			SPV_BLURRING,
			// num
			SPV_NUM ///< The number of all buildin variables
	};

	/// Set the uniform using this visitor
	class UsrDefVarVisitor: public boost::static_visitor<>
	{
		public:
			const MaterialRuntimeVariable& udvr;
			const Renderer& r;
			const PassLevelKey& key;
			uint& texUnit;

			UsrDefVarVisitor(const MaterialRuntimeVariable& udvr,
				const Renderer& r, const PassLevelKey& key, uint& texUnit);

			/// Functor
			template<typename Type>
			void operator()(const Type& x) const;

			/// Functor
			void operator()(const TextureResourcePointer* x) const;
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
		const PassLevelKey& key,
		const Transform& nodeWorldTransform,
		const Camera& cam,
		const Renderer& r,
		float blurring);

	void setTheBuildins();
};


} // end namespace


#endif
