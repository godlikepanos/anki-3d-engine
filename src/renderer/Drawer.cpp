// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Drawer.h"
#include "anki/renderer/Ms.h"
#include "anki/resource/ShaderResource.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Counters.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
static const U MATERIAL_BLOCK_MAX_SIZE = 1024 * 4;

/// Visitor that sets a uniform
class SetupRenderableVariableVisitor
{
public:
	// Used to get the visible spatials
	Ptr<VisibleNode> m_visibleNode;

	Ptr<RenderComponent> m_renderable; ///< To get the transforms
	Ptr<RenderComponentVariable> m_rvar;
	Ptr<const FrustumComponent> m_fr;
	Ptr<RenderableDrawer> m_drawer;
	U8 m_instanceCount;
	CommandBufferPtr m_cmdBuff;
	Array<U8, MATERIAL_BLOCK_MAX_SIZE> m_tempUniformBuffer;

	F32 m_flod;

	SetupRenderableVariableVisitor(RenderableDrawer* drawer)
		: m_drawer(drawer)
	{}

	HeapAllocator<U8> getAllocator() const
	{
		return m_drawer->m_r->getAllocator();
	}

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const MaterialVariable& mtlVar,
		const T* value, U32 size)
	{
		mtlVar.writeShaderBlockMemory<T>(
			value,
			size,
			&m_tempUniformBuffer[0],
			&m_tempUniformBuffer[0] + MATERIAL_BLOCK_MAX_SIZE);
	}

	template<typename TRenderableVariableTemplate>
	Error visit(const TRenderableVariableTemplate& rvar)
	{
		typedef typename TRenderableVariableTemplate::Type DataType;
		const MaterialVariable& glvar = rvar.getMaterialVariable();

		// Array size
		U arraySize;
		if(rvar.isInstanced())
		{
			arraySize = std::min<U>(m_instanceCount, glvar.getArraySize());
		}
		else
		{
			arraySize = glvar.getArraySize();
		}

		// Set uniform
		//
		Bool hasWorldTrfs = m_renderable->getHasWorldTransforms();
		const Mat4& vp = m_fr->getViewProjectionMatrix();
		const Mat4& v = m_fr->getViewMatrix();

		switch(rvar.getBuildinId())
		{
		case BuildinMaterialVariableId::NO_BUILDIN:
			uniSet<DataType>(glvar, rvar.begin(), arraySize);
			break;
		case BuildinMaterialVariableId::MVP_MATRIX:
			if(hasWorldTrfs)
			{
				Mat4* mvp = m_drawer->m_r->getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					mvp[i] = vp * Mat4(worldTrf);
				}

				uniSet(glvar, &mvp[0], arraySize);
			}
			else
			{
				ANKI_ASSERT(arraySize == 1 && "Shouldn't instance that one");
				uniSet(glvar, &vp, 1);
			}
			break;
		case BuildinMaterialVariableId::MV_MATRIX:
			{
				ANKI_ASSERT(hasWorldTrfs);
				Mat4* mv = m_drawer->m_r->getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					mv[i] = v * Mat4(worldTrf);
				}

				uniSet(glvar, &mv[0], arraySize);
			}
			break;
		case BuildinMaterialVariableId::VP_MATRIX:
			uniSet(glvar, &vp, 1);
			break;
		case BuildinMaterialVariableId::NORMAL_MATRIX:
			if(hasWorldTrfs)
			{
				Mat3* normMats =
					m_drawer->m_r->getFrameAllocator().
					newInstance<Mat3>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					Mat4 mv = v * Mat4(worldTrf);
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				uniSet(glvar, &normMats[0], arraySize);
			}
			else
			{
				ANKI_ASSERT(arraySize == 1
					&& "Having that instanced doesn't make sense");

				Mat3 normMat = v.getRotationPart();

				uniSet(glvar, &normMat, 1);
			}
			break;
		case BuildinMaterialVariableId::BILLBOARD_MVP_MATRIX:
			{
				// Calc the billboard rotation matrix
				Mat3 rot =
					m_fr->getViewMatrix().getRotationPart().getTransposed();

				Mat4* bmvp =
					m_drawer->m_r->getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform trf;
					m_renderable->getRenderWorldTransform(i, trf);
					trf.setRotation(Mat3x4(rot));
					bmvp[i] = vp * Mat4(trf);
				}

				uniSet(glvar, &bmvp[0], arraySize);
			}
			break;
		case BuildinMaterialVariableId::MAX_TESS_LEVEL:
			{
				F32 maxtess =
					rvar.RenderComponentVariable:: template operator[]<F32>(0);
				F32 tess = 0.0;

				if(m_flod >= 1.0)
				{
					tess = 1.0;
				}
				else
				{
					tess = maxtess - m_flod * maxtess;
					tess = std::max(tess, 1.0f);
				}

				uniSet(glvar, &tess, 1);
			}
			break;
		case BuildinMaterialVariableId::BLURRING:
			{
				F32 blurring = 0.0;
				uniSet(glvar, &blurring, 1);
			}
			break;
		case BuildinMaterialVariableId::MS_DEPTH_MAP:
			{
				auto unit = glvar.getTextureUnit();

				m_drawer->m_r->getMs().getDepthRt().bind(m_cmdBuff, unit);
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		return ErrorCode::NONE;
	}
};

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePtr>(
	const MaterialVariable& mtlvar,
	const TextureResourcePtr* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	TexturePtr tex = (*values)->getGlTexture();
	auto unit = mtlvar.getTextureUnit();

	tex.bind(m_cmdBuff, unit);
}

//==============================================================================
RenderableDrawer::RenderableDrawer()
{}

//==============================================================================
RenderableDrawer::~RenderableDrawer()
{}

//==============================================================================
Error RenderableDrawer::create(Renderer* r)
{
	m_r = r;
	m_variableVisitor.reset(
		m_r->getAllocator().newInstance<SetupRenderableVariableVisitor>(this));
	return ErrorCode::NONE;
}

//==============================================================================
void RenderableDrawer::setupUniforms(
	VisibleNode& visibleNode,
	RenderComponent& renderable,
	FrustumComponent& fr,
	F32 flod)
{
	const Material& mtl = renderable.getMaterial();

	ANKI_ASSERT(mtl.getDefaultBlockSize() < MATERIAL_BLOCK_MAX_SIZE);

	// Call the visitor
	//
	m_variableVisitor->m_visibleNode = &visibleNode;
	m_variableVisitor->m_renderable = &renderable;
	m_variableVisitor->m_fr = &fr;
	m_variableVisitor->m_instanceCount = visibleNode.m_spatialsCount;
	m_variableVisitor->m_cmdBuff = m_cmdBuff;
	m_variableVisitor->m_flod = flod;

	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		m_variableVisitor->m_rvar = rvar;
		Error err = rvar->acceptVisitor(*m_variableVisitor);
		(void)err;
	}

	// Update the uniforms
	//
	m_cmdBuff.updateDynamicUniforms(
		&m_variableVisitor->m_tempUniformBuffer[0], mtl.getDefaultBlockSize());
}

//==============================================================================
Error RenderableDrawer::render(SceneNode& frsn, VisibleNode& visibleNode)
{
	RenderingBuildData build;

	// Get components
	FrustumComponent& fr = frsn.getComponent<FrustumComponent>();
	RenderComponent& renderable =
		visibleNode.m_node->getComponent<RenderComponent>();
	const Material& mtl = renderable.getMaterial();

	// Calculate the key
	RenderingKey key;

	Vec4 camPos = fr.getFrustumOrigin();

	F32 dist = (visibleNode.m_node->getComponent<SpatialComponent>().
		getSpatialOrigin() - camPos).getLength();
	F32 flod = m_r->calculateLod(dist);
	build.m_key.m_lod = flod;
	build.m_key.m_pass = m_pass;
	build.m_key.m_tessellation =
		m_r->getTessellationEnabled()
		&& mtl.getTessellationEnabled()
		&& build.m_key.m_lod == 0;

	if(m_pass == Pass::SM)
	{
		build.m_key.m_tessellation = false;
	}

	// Enqueue uniform state updates
	setupUniforms(visibleNode, renderable, fr, flod);

	// Enqueue vertex, program and drawcall
	build.m_subMeshIndicesArray = &visibleNode.m_spatialIndices[0];
	build.m_subMeshIndicesCount = visibleNode.m_spatialsCount;
	build.m_cmdb = m_cmdBuff;

	Error err = renderable.buildRendering(build);
	if(err)
	{
		return err;
	}

	return ErrorCode::NONE;
}

//==============================================================================
void RenderableDrawer::prepareDraw(RenderingStage stage, Pass pass,
	CommandBufferPtr& cmdBuff)
{
	// Set some numbers
	m_stage = stage;
	m_pass = pass;
	m_cmdBuff = cmdBuff;
}

//==============================================================================
void RenderableDrawer::finishDraw()
{
	// Release the job chain
	m_cmdBuff = CommandBufferPtr();
}

}  // end namespace anki
