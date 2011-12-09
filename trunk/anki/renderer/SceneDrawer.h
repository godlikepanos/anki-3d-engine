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
class ShaderProgramUniformVariable;

class Renderer;


/// It includes all the functions to render a RenderableNode
class SceneDrawer
{
public:
	/// The one and only constructor
	SceneDrawer(const Renderer& r_)
		: r(r_)
	{}

	void renderRenderableNode(const Camera& cam,
		uint pass, RenderableNode& renderable) const;

private:
	/// Standard attribute variables that are acceptable inside the
	/// ShaderProgram
	enum Buildins
	{
		// Matrices
		B_MODEL_MAT,
		B_VIEW_MAT,
		B_PROJECTION_MAT,
		B_MODEL_VIEW_MAT,
		B_VIEW_PROJECTION_MAT,
		B_NORMAL_MAT,
		B_MODEL_VIEW_PROJECTION_MAT,
		// FAIs (for materials in blending stage)
		B_MS_NORMAL_FAI,
		B_MS_DIFFUSE_FAI,
		B_MS_SPECULAR_FAI,
		B_MS_DEPTH_FAI,
		B_IS_FAI,
		B_PPS_PRE_PASS_FAI,
		B_PPS_POST_PASS_FAI,
		// Other
		B_RENDERER_SIZE,
		B_SCENE_AMBIENT_COLOR,
		B_BLURRING,
		// num
		B_NUM ///< The number of all buildin variables
	};

	/// Set the uniform using this visitor
	class SetUniformVisitor: public boost::static_visitor<>
	{
	public:
		const ShaderProgramUniformVariable& uni;
		uint& texUnit;

		SetUniformVisitor(const ShaderProgramUniformVariable& uni,
			uint& texUnit);

		template<typename Type>
		void operator()(const Type& x) const;
	};

	static boost::array<const char*, B_NUM> buildinsTxt;
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
		const PassLevelKey& key,
		const Transform& nodeWorldTransform,
		const Transform& prevNodeWorldTransform,
		const Camera& cam,
		const Renderer& r,
		MaterialRuntime& mtlr);

	/// For code duplication
	static void tryCalcModelViewMat(const Mat4& modelMat, const Mat4& viewMat,
		bool& calculated, Mat4& modelViewMat);
};


} // end namespace


#endif
