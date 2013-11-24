#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Visibility.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
static const U UNIFORM_BLOCK_MAX_SIZE = 1024 * 12;

//==============================================================================
#if ANKI_ENABLE_COUNTERS
static U64 countVerts(
	const Array<U32, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesCount, 
	I drawcallCount)
{
	U64 sum = 0;
	while(--drawcallCount >= 0)
	{
		sum += indicesCount[drawcallCount];
	}
	return sum;
}
#endif

//==============================================================================
/// Visitor that sets a uniform
/// Align it because the clientBlock will store SIMD data
ANKI_ATTRIBUTE_ALIGNED(struct, 16) SetupRenderableVariableVisitor
{
	Array<U8, UNIFORM_BLOCK_MAX_SIZE> clientBlock;
	const FrustumComponent* fr = nullptr;
	Renderer* r = nullptr;
	RenderComponent* renderable = nullptr;
	RenderComponentVariable* rvar = nullptr;
	const ShaderProgramUniformVariable* uni;
	F32 flod;
	Drawcall* dc = nullptr;

	// Used for 
	VisibleNode* visibleNode;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const ShaderProgramUniformVariable& uni,
		const T* value, U32 size)
	{
		ANKI_ASSERT(0);
	}

	template<typename TRenderableVariableTemplate>
	void visit(TRenderableVariableTemplate& x)
	{
		const U32 drawCount = std::max(dc->drawCount, dc->instancesCount);
		const U32 uniArrSize = uni->getSize();
		U32 size = std::min(drawCount, uniArrSize);

		// Set uniform
		//
		Bool hasWorldTrfs = renderable->getHasWorldTransforms();
		const Mat4& vp = fr->getViewProjectionMatrix();
		const Mat4& v = fr->getViewMatrix();

		switch(x.getBuildinId())
		{
		case BMV_NO_BUILDIN:
			uniSet<typename TRenderableVariableTemplate::Type>(
				*uni, x.get(), x.getArraySize());
			break;
		case BMV_MVP_MATRIX:
			if(hasWorldTrfs)
			{
				Array<Mat4, ANKI_MAX_INSTANCES> mvp;

				for(U i = 0; i < size; i++)
				{
					Transform worldTrf;

					renderable->getRenderWorldTransform(
						visibleNode->getSpatialIndex(i), worldTrf);

					mvp[i] = vp * Mat4(worldTrf);
				}

				uniSet(*uni, &mvp[0], size);
			}
			else
			{
				ANKI_ASSERT(size == 1 && "Shouldn't instance that one");
				uniSet(*uni, &vp, 1);
			}
			break;
		case BMV_MV_MATRIX:
			{
				ANKI_ASSERT(hasWorldTrfs);
				Array<Mat4, ANKI_MAX_INSTANCES> mv;

				for(U i = 0; i < size; i++)
				{
					Transform worldTrf;

					renderable->getRenderWorldTransform(
						visibleNode->getSpatialIndex(i), worldTrf);

					mv[i] = v * Mat4(worldTrf);
				}

				uniSet(*uni, &mv[0], size);
			}
			break;
		case BMV_VP_MATRIX:
			uniSet(*uni, &vp, 1);
			break;
		case BMV_NORMAL_MATRIX:
			if(hasWorldTrfs)
			{
				Array<Mat3, ANKI_MAX_INSTANCES> normMats;

				for(U i = 0; i < size; i++)
				{
					Transform worldTrf;

					renderable->getRenderWorldTransform(
						visibleNode->getSpatialIndex(i), worldTrf);

					Mat4 mv = v * Mat4(worldTrf);
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				uniSet(*uni, &normMats[0], size);
			}
			else
			{
				ANKI_ASSERT(uniArrSize == 1
					&& "Having that instanced doesn't make sense");

				Mat3 normMat = v.getRotationPart();

				uniSet(*uni, &normMat, 1);
			}
			break;
		case BMV_BILLBOARD_MVP_MATRIX:
			{
				// Calc the billboard rotation matrix
				Mat3 rot =
					fr->getViewMatrix().getRotationPart().getTransposed();

				Array<Mat4, ANKI_MAX_INSTANCES> bmvp;

				for(U i = 0; i < size; i++)
				{
					Transform trf;
					renderable->getRenderWorldTransform(i, trf);
					trf.setRotation(rot);
					bmvp[i] = vp * Mat4(trf);
				}

				uniSet(*uni, &bmvp[0], size);
			}
			break;
		case BMV_MAX_TESS_LEVEL:
			{
				RenderComponentVariable& tmp = x;
				F32 maxtess = tmp.getValues<F32>()[0];
				F32 tess;
				
				if(flod >= 1.0)
				{
					tess = 1.0;
				}
				else
				{
					tess = maxtess - flod * maxtess;
					tess = std::max(tess, 1.0f);
				}
				
				uniSet(*uni, &tess, 1);
			}
			break;
		case BMV_BLURRING:
			{
				F32 blurring = 0.0;
				uniSet(*uni, &blurring, 1);
			}
			break;
		case BMV_MS_DEPTH_MAP:
			uni->set(r->getMs().getDepthFai());
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}
};

/// Specialize the material accepted types. The un-specialized will be used for
/// all Property types like strings, we don't need strings in our case
#define TEMPLATE_SPECIALIZATION(type) \
	template<> \
	void SetupRenderableVariableVisitor::uniSet<type>( \
		const ShaderProgramUniformVariable& uni, const type* values, \
		U32 size) \
	{ \
		if(uni.getUniformBlock()) \
		{ \
			uni.setClientMemory(&clientBlock[0], \
				sizeof(clientBlock), \
				values, size); \
		} \
		else \
		{ \
			uni.set(values, size); \
		} \
	}

TEMPLATE_SPECIALIZATION(F32)
TEMPLATE_SPECIALIZATION(Vec2)
TEMPLATE_SPECIALIZATION(Vec3)
TEMPLATE_SPECIALIZATION(Vec4)
TEMPLATE_SPECIALIZATION(Mat3)
TEMPLATE_SPECIALIZATION(Mat4)

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni, 
	const TextureResourcePointer* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	const Texture* tex = values->get();
	uni.set(*tex);
}

//==============================================================================
void RenderableDrawer::setupShaderProg(const PassLodKey& key_,
	const FrustumComponent& fr, const ShaderProgram &prog,
	RenderComponent& renderable, 
	VisibleNode& visibleNode,
	F32 flod,
	Drawcall* dc)
{
	prog.bind();
	
	SetupRenderableVariableVisitor vis;

	vis.fr = &fr;
	vis.renderable = &renderable;
	vis.r = r;
	vis.visibleNode = &visibleNode;
	vis.flod = flod;
	vis.dc = dc;

	PassLodKey key(key_.pass,
		std::min(key_.level,
		U8(renderable.getMaterial().getLevelsOfDetail() - 1)));

	// Set the uniforms
	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		const ShaderProgramUniformVariable* uni =
			rvar->tryFindShaderProgramUniformVariable(key);

		if(uni)
		{
			vis.rvar = rvar;
			vis.uni = uni;
			rvar->acceptVisitor(vis);
		}
	}

	// Write the block
	/*const ShaderProgramUniformBlock* block =
		renderable.getMaterial().getCommonUniformBlock();
	if(block)
	{
		ANKI_ASSERT(block->getSize() <= UNIFORM_BLOCK_MAX_SIZE);
		ANKI_ASSERT(block->getBinding() == 0);
		renderable.getUbo().write(&vis.clientBlock[0]);
		renderable.getUbo().setBinding(0);
	}*/
}

//==============================================================================
void RenderableDrawer::render(SceneNode& frsn, RenderingStage stage,
	Pass pass, VisibleNode& visibleNode)
{
	// Get components
	FrustumComponent& fr = frsn.getComponent<FrustumComponent>();
	RenderComponent& renderable = 
		visibleNode.node->getComponent<RenderComponent>();

	// Calculate the key
	Vec3 camPos = fr.getFrustumOrigin();

	F32 dist = (visibleNode.node->getComponent<SpatialComponent>().
		getSpatialOrigin() - camPos).getLength();
	F32 lod = r->calculateLod(dist);

	PassLodKey key(pass, lod);

	// Blending
	GlState& gl = GlStateSingleton::get();
	const Material& mtl = renderable.getMaterial();

	Bool blending = mtl.isBlendingEnabled();

	if(!blending)
	{
		if(stage == RS_BLEND)
		{
			return;
		}
	}
	else
	{
		if(stage != RS_BLEND)
		{
			return;
		}

		gl.setBlendFunctions(
			mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}

	gl.enable(GL_BLEND, blending);

#if ANKI_GL == ANKI_GL_DESKTOP
	gl.setPolygonMode(mtl.getWireframe() ? GL_LINE : GL_FILL);
#endif

	// Get rendering useful drawing stuff
	const ShaderProgram* prog;
	const Vao* vao;
	Drawcall dc;

	renderable.getRenderingData(
		key, visibleNode.spatialIndices, visibleNode.spatialsCount, // in
		vao, prog, dc); //out

	// Setup shader
	setupShaderProg(
		key, fr, *prog, renderable, visibleNode, lod, &dc);

	// Render
	ANKI_ASSERT(vao->getAttachmentsCount() > 1);
	vao->bind();

	// Tessellation
#if ANKI_GL == ANKI_GL_DESKTOP
	if(mtl.getTessellation())
	{
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		dc.primitiveType = GL_PATCHES;
	}
#endif

	// Start drawcall
	dc.enque();

	ANKI_COUNTER_INC(C_RENDERER_DRAWCALLS_COUNT, (U64)1);
	ANKI_COUNTER_INC(C_RENDERER_VERTICES_COUNT, 
		countVerts(indicesCountArray, (I)drawcallCount));
}

}  // end namespace anki
