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

struct SetBuildinIdVisitor
{
	Bool setted;

	template<typename TProp>
	void visit(TProp& x)
	{
		MaterialVariableProperty<typename TProp::Value>& mprop =
			static_cast<MaterialVariableProperty<typename TProp::Value>&>(x);

		const MaterialVariable& mv = mprop.getMaterialVariable();

		if(mprop.getBuildinId() == BI_UNITIALIZED)
		{
			const std::string& name = mv.getName();

			for(U i = 0; i < buildinNames.size(); i++)
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

			setted = true;
		}

		setted = false
	}
};

//==============================================================================
static const UNIFORM_BLOCK_MAX_SIZE = 256;

/// Visitor that sets a uniform
struct SetupMaterialVariableVisitor
{
	PassLevelKey key;
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;
	Array<U8, UNIFORM_BLOCK_MAX_SIZE> clientBlock;

	/// Set a uniform in a client block
	template<typename T>
	static void uniSet(const ShaderProgramUniformVariable& uni, const T& value)
	{
		uni.setClientMemory(&clientBlock[0], UNIFORM_BLOCK_MAX_SIZE, &value, 1);
	}

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

		// Sanity check
		//
		ANKI_ASSERT(mprop.getBuildinId() != BI_UNITIALIZED);
		if(!mv.hasValue() && mprop.getBuildinId() == BT_NO_BUILDIN)
		{
			ANKI_LOGW("Material variable no building and not initialized: "
				<< mv.getName());
		}

		// Set uniform
		//
		const Transform* rwtrf = renderable->getRenderableWorldTransform();

		Mat4 mMat = (rwtrf) ? Mat4(*rwtrf) : Mat4::getIdentity();
		const Mat4& vpMat = fr->getViewProjectionMatrix();
		Mat4 mvpMat = vpMat * mMat;

		Mat4 mvMat;
		Bool mvMatCalculated = false; // Opt

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

// Texture specialization
template<>
void uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni,
	const TextureResourcePointer& x,
	void*)
{
	const Texture* tex = x.get();
	uni.set(*tex);
}

//==============================================================================
void RenderableDrawer::setBuildinIds(Renderable& renderable)
{
	SetBuildinIdVisitor vis;

	for(auto it = renderable.getPropertiesBegin();
		it != renderable.getPropertiesEnd(); ++it)
	{
		PropertyBase* prop = *it;

		pbase->acceptVisitor(vis);

		if(!vis.setted)
		{
			return;
		}
	}
}

//==============================================================================
void RenderableDrawer::setupShaderProg(
	const PassLevelKey& key,
	const Frustumable& fr,
	Renderable& renderable)
{
	setBuildinIds(renderable);

	const Material& mtl = renderable.getMaterial();
	const ShaderProgram& sprog = mtl.findShaderProgram(key);

	sprog.bind();
	
	SetupMaterialVariableVisitor vis;

	vis.fr = &fr;
	vis.key = key;
	vis.renderable = &renderable;
	vis.r = r;

	for(auto it = renderable.getPropertiesBegin();
		it != renderable.getPropertiesEnd(); ++it)
	{
		PropertyBase* pbase = *it;
		pbase->acceptVisitor(vis);
	}

	const ShaderProgramUniformBlock* block = mtl->getUniformBlock();
	if(block)
	{
		ANKI_ASSERT(block->getSize() <= UNIFORM_BLOCK_MAX_SIZE);
		renderable.getUbo().write(&vis.clientBlock[0]);
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
