#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
static const U UNIFORM_BLOCK_MAX_SIZE = 1024 * 12;

//==============================================================================
#if ANKI_ENABLE_COUNTERS
static U64 countVerts(U32* indicesCount, I primCount)
{
	U64 sum = 0;
	while(--primCount >= 0)
	{
		sum += indicesCount[primCount];
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

	// Used for 
	const U32* subSpatialIndices = nullptr;
	U32 subSpatialIndicesCount = 0;

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
		const U32 instancesCount = renderable->getRenderInstancesCount();
		const U32 uniArrSize = uni->getSize();
		U32 size = std::min(instancesCount, uniArrSize);

		// Set uniform
		//
		const Transform* trfs = renderable->getRenderWorldTransforms();
		const Mat4& vp = fr->getViewProjectionMatrix();
		const Mat4& v = fr->getViewMatrix();

		switch(x.getBuildinId())
		{
		case BMV_NO_BUILDIN:
			uniSet<typename TRenderableVariableTemplate::Type>(
				*uni, x.get(), x.getArraySize());
			break;
		case BMV_MVP_MATRIX:
			if(trfs)
			{
				Array<Mat4, ANKI_MAX_INSTANCES> mvp;

				if(subSpatialIndicesCount == 0)
				{
					for(U i = 0; i < size; i++)
					{
						mvp[i] = vp * Mat4(trfs[i]);
					}
				}
				else
				{
					for(size = 0; size < subSpatialIndicesCount; size++)
					{
						ANKI_ASSERT(size < instancesCount 
							&& size < uniArrSize);
						mvp[size] = vp * Mat4(trfs[subSpatialIndices[size]]);
					}
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
				ANKI_ASSERT(trfs != nullptr);
				Array<Mat4, ANKI_MAX_INSTANCES> mv;

				if(subSpatialIndicesCount == 0)
				{
					for(U i = 0; i < size; i++)
					{
						mv[i] = v * Mat4(trfs[i]);
					}
				}
				else
				{
					for(size = 0; size < subSpatialIndicesCount; size++)
					{
						ANKI_ASSERT(size < instancesCount 
							&& size < uniArrSize);

						mv[size] = v * Mat4(trfs[subSpatialIndices[size]]);
					}
				}

				uniSet(*uni, &mv[0], size);
			}
			break;
		case BMV_VP_MATRIX:
			uniSet(*uni, &vp, 1);
			break;
		case BMV_NORMAL_MATRIX:
			if(trfs)
			{
				Array<Mat3, ANKI_MAX_INSTANCES> normm;

				if(subSpatialIndicesCount == 0)
				{
					for(U i = 0; i < size; i++)
					{
						Mat4 mv = v * Mat4(trfs[i]);
						normm[i] = mv.getRotationPart();
					}
				}
				else
				{
					for(size = 0; size < subSpatialIndicesCount; size++)
					{
						Mat4 mv = v * Mat4(trfs[subSpatialIndices[size]]);
						normm[size] = mv.getRotationPart();
					}
				}

				uniSet(*uni, &normm[0], size);
			}
			else
			{
				ANKI_ASSERT(uniArrSize == 1
					&& "Having that instanced doesn't make sense");

				Mat3 norm = v.getRotationPart();

				uniSet(*uni, &norm, 1);
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
					Transform trf = trfs[i];
					trf.setRotation(rot);
					bmvp[i] = vp * Mat4(trf);
				}

				uniSet(*uni, &bmvp[0], size);
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
void RenderableDrawer::setupShaderProg(const PassLevelKey& key_,
	const FrustumComponent& fr, const ShaderProgram &prog,
	RenderComponent& renderable, 
	U32* subSpatialIndices, U subSpatialIndicesCount)
{
	prog.bind();
	
	SetupRenderableVariableVisitor vis;

	vis.fr = &fr;
	vis.renderable = &renderable;
	vis.r = r;
	vis.subSpatialIndices = subSpatialIndices;
	vis.subSpatialIndicesCount = subSpatialIndicesCount;

	PassLevelKey key(key_.pass,
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
	Pass pass, SceneNode& rsn, U32* subSpatialIndices,
	U subSpatialIndicesCount)
{
	ANKI_ASSERT(frsn.getFrustumComponent());
	FrustumComponent& fr = *frsn.getFrustumComponent();
	RenderComponent* renderable = rsn.getRenderComponent();
	ANKI_ASSERT(renderable);

	// Instancing
	U32 instancesCount = renderable->getRenderInstancesCount();

	if(ANKI_UNLIKELY(instancesCount < 1))
	{
		return;
	}

	GlState& gl = GlStateSingleton::get();

	// Blending
	const Material& mtl = renderable->getMaterial();

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

	gl.setPolygonMode(mtl.getWireframe() ? GL_LINE : GL_FILL);

	// Calculate the LOD
	Vec3 camPos = fr.getFrustumOrigin();

	F32 dist = 
		(rsn.getSpatialComponent()->getSpatialOrigin() - camPos).getLength();
	U8 lod = r->calculateLod(dist);

	PassLevelKey key(pass, lod);

	// Get rendering useful stuff
	const ShaderProgram* prog;
	const Vao* vao;
	U32 indicesCountArray[ANKI_MAX_MULTIDRAW_PRIMITIVES];
	const void* indicesOffsetArray[ANKI_MAX_MULTIDRAW_PRIMITIVES];
#if ANKI_DEBUG
	memset(indicesCountArray, 0, sizeof(indicesCountArray));
	memset(indicesOffsetArray, 0, sizeof(indicesOffsetArray));
#endif

	U32 primCount = 1;

	const ModelPatchBase& resource = renderable->getModelPatchBase();
	if(subSpatialIndicesCount == 0 || resource.getSubMeshesCount() == 0)
	{
		// No multimesh

		resource.getRenderingData(
			key, vao, prog, indicesCountArray[0]);

		indicesOffsetArray[0] = nullptr;
	}
	else
	{
		// It's a multimesh

		resource.getRenderingDataSub(
			key, vao, prog, 
			subSpatialIndices, subSpatialIndicesCount,
			indicesCountArray, indicesOffsetArray, primCount);
	}

	// Hack the instances count
	if(subSpatialIndicesCount > 0 && instancesCount > 1)
	{
		instancesCount = subSpatialIndicesCount;
	}

	// Setup shader
	setupShaderProg(
		key, fr, *prog, *renderable, subSpatialIndices, subSpatialIndicesCount);

	// Render
	ANKI_ASSERT(vao->getAttachmentsCount() > 1);
	vao->bind();

	// Draw call
	Drawcall dc;

	dc.primitiveType = GL_TRIANGLES;
	dc.indicesType = GL_UNSIGNED_SHORT;
	dc.instancesCount = instancesCount;
	dc.indicesCountArray = (GLsizei*)indicesCountArray;
	dc.offsetsArray = indicesOffsetArray;
	dc.primCount = primCount;

	dc.enque();
	ANKI_COUNTER_INC(C_RENDERER_DRAWCALLS_COUNT, (U64)1);
	ANKI_COUNTER_INC(C_RENDERER_VERTICES_COUNT, 
		countVerts(indicesCountArray, (I)primCount));
}

}  // end namespace anki
