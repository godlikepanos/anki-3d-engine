#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/Frustumable.h"
#include "anki/resource/Material.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================

enum BuildinId
{
	BI_UNITIALIZED = 0,
	BT_NO_BUILDIN,
	BI_MODEL_VIEW_PROJECTION_MATRIX,
	BI_MODEL_VIEW_MATRIX,
	BI_NORMAL_MATRIX,
	BI_BLURRING,
	BI_COUNT
};

static Array<const char*, BI_COUNT - 2> buildinNames = {{
	"modelViewProjectionMat",
	"modelViewMat",
	"normalMat",
	"blurring"
}};

static const UNIFORM_BLOCK_MAX_SIZE = 256;

template<typename T>
static void uniSet(const ShaderProgramUniformVariable& uni, const T& x)
{
	(void)uni;
	(void)x;
	ANKI_ASSERT(0);
}

#define TEMPLATE_SPECIALIZATION(type) \
	template<> \
	void uniSet<type>(const ShaderProgramUniformVariable& uni, const type& x) \
	{ \
		uni.set(x); \
	}

TEMPLATE_SPECIALIZATION(float)
TEMPLATE_SPECIALIZATION(Vec2)
TEMPLATE_SPECIALIZATION(Vec3)
TEMPLATE_SPECIALIZATION(Vec4)
TEMPLATE_SPECIALIZATION(Mat3)
TEMPLATE_SPECIALIZATION(Mat4)

// Texture specialization
template<>
void uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni,
	const TextureResourcePointer& x)
{
	const Texture* tex = x.get();
	uni.set(*tex);
}

/// XXX
struct SetupMaterialVariableVisitor
{
	PassLevelKey key;
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;
	Array<U8, UNIFORM_BLOCK_MAX_SIZE> clientBlock;

	template<typename TProp>
	void visit(TProp& x)
	{
		MaterialVariableProperty<typename TProp::Value>& mprop =
			static_cast<MaterialVariableProperty<typename TProp::Value>&>(x);

		const MaterialVariable& mv = mprop.getMaterialVariable();

		const ShaderProgramUniformVariable* uni =
			mv.findShaderProgramUniformVariable(key);

		if(!uni)
		{
			return;
		}

		// Set buildin id
		//
		if(mprop.getBuildinId() == BI_UNITIALIZED)
		{
			const std::string name = mv.getName();

			for(uint32_t i = 0; i < buildinNames.size(); i++)
			{
				if(name == buildinNames[i])
				{
					mprop.setBuildinId(i + 2);
					break;
				}
			}

			if(mprop.getBuildinId() == BI_UNITIALIZED)
			{
				mprop.setBuildinId(BT_NO_BUILDIN);
			}
		}

		// Sanity check
		//
		/*if(!mv.getInitialized() && mprop.getBuildinId() == BT_NO_BUILDIN)
		{
			ANKI_LOGW("Material variable no building and not initialized: "
				<< mv.getName());
		}*/

		// Set uniform
		//
		const Transform* rwtrf = renderable->getRenderableWorldTransform();

		Mat4 mMat = (rwtrf) ? Mat4(*rwtrf) : Mat4::getIdentity();
		const Mat4& vpMat = fr->getViewProjectionMatrix();
		Mat4 mvpMat = vpMat * mMat;

		Mat4 mvMat;
		bool mvMatCalculated = false; // Opt

		switch(mprop.getBuildinId())
		{
		case BT_NO_BUILDIN:
			uniSet(*uni, mprop.getValue());
			break;
		case BI_MODEL_VIEW_PROJECTION_MATRIX:
			uniSet(*uni, mvpMat);
			break;
		case BI_MODEL_VIEW_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = mMat * fr->getViewMatrix();
				mvMatCalculated = true;
			}
			uniSet(*uni, mvMat);
			break;
		case BI_NORMAL_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = mMat * fr->getViewMatrix();
				mvMatCalculated = true;
			}
			uniSet(*uni, mvMat.getRotationPart());
			break;
		case BI_BLURRING:
			uniSet(*uni, 0.0);
			break;
		}
	}
};

//==============================================================================
void RenderableDrawer::setupShaderProg(
	const PassLevelKey& key,
	const Frustumable& fr,
	Renderable& renderable)
{
	const Material& mtl = renderable.getMaterial();
	const ShaderProgram& sprog = mtl.findShaderProgram(key);
	Array<U8, UNIFORM_BLOCK_MAX_SIZE> clientBlock;

	/*if(mtl.getDepthTestingEnabled())
	{
		GlStateSingleton::get().enable(GL_DEPTH_TEST);
	}
	else
	{
		GlStateSingleton::get().disable(GL_DEPTH_TEST);
	}*/

	sprog.bind();
	
	SetupMaterialVariableVisitor vis;

	vis.fr = &fr;
	vis.key = key;
	vis.renderable = &renderable;
	vis.r = r;
	vis.clientBlock = &clientBlock;

	for(auto it = renderable.getPropertiesBegin();
		it != renderable.getPropertiesEnd(); ++it)
	{
		PropertyBase* pbase = *it;
		pbase->acceptVisitor(vis);
	}

	if(mtl->getUniformBlock())
	{
		renderable.getUbo().setBindingPoint(0);
	}
}

//==============================================================================
void RenderableDrawer::render(const Frustumable& fr, uint pass,
	Renderable& renderable)
{
	/*float dist = (node.getWorldTransform().getOrigin() -
		cam.getWorldTransform().getOrigin()).getLength();
	uint lod = std::min(r.calculateLod(dist), mtl.getLevelsOfDetail() - 1);*/

	PassLevelKey key(pass, 0);

	// Setup shader
	setupShaderProg(key, fr, renderable);

	// Render
	U32 indecesNum = renderable.getModelPatchBase().getIndecesNumber(0);

	const Vao& vao = renderable.getModelPatchBase().getVao(key);
	ANKI_ASSERT(vao.getAttachmentsCount() > 1);
	vao.bind();
	glDrawElements(GL_TRIANGLES, indecesNum, GL_UNSIGNED_SHORT, 0);
	vao.unbind();
}

}  // end namespace anki
