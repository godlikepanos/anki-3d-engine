// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/UiInterfaceImpl.h"
#include "anki/resource/ResourceManager.h"

namespace anki {

//==============================================================================
struct Vertex
{
	Vec2 m_pos;
	Vec2 m_uv;
	Array<U8, 4> m_color;
};

//==============================================================================
UiInterfaceImpl::UiInterfaceImpl(UiAllocator alloc)
	: UiInterface(alloc)
{}

//==============================================================================
UiInterfaceImpl::~UiInterfaceImpl()
{}

//==============================================================================
Error UiInterfaceImpl::init(GrManager* gr, ResourceManager* rc)
{
	// Load shaders
	ANKI_CHECK(rc->loadResource("shaders/UiLines.vert.glsl",
		m_stages[StageId::LINES].m_vShader));

	ANKI_CHECK(rc->loadResource("shaders/UiLines.frag.glsl",
		m_stages[StageId::LINES].m_fShader));

	// Init pplines
	PipelineInitializer ppinit;
	ppinit.m_vertex.m_bindingCount = 1;
	ppinit.m_vertex.m_bindings[0].m_stride = sizeof(Vertex);
	ppinit.m_vertex.m_attributeCount = 3;
	ppinit.m_vertex.m_attributes[0].m_format =
		PixelFormat(ComponentFormat::R32G32, TransformFormat::FLOAT);
	ppinit.m_vertex.m_attributes[0].m_offset = 0;
	ppinit.m_vertex.m_attributes[1].m_format =
		PixelFormat(ComponentFormat::R32G32, TransformFormat::FLOAT);
	ppinit.m_vertex.m_attributes[1].m_offset = sizeof(Vec2);
	ppinit.m_vertex.m_attributes[2].m_format =
		PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
	ppinit.m_vertex.m_attributes[2].m_offset = sizeof(Vec2) * 2;

	ppinit.m_inputAssembler.m_topology = PrimitiveTopology::LINES;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format =
		PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	ppinit.m_color.m_attachments[0].m_srcBlendMethod = BlendMethod::SRC_ALPHA;
	ppinit.m_color.m_attachments[0].m_dstBlendMethod =
		BlendMethod::ONE_MINUS_SRC_ALPHA;

	ppinit.m_shaders[U(ShaderType::VERTEX)] =
		m_stages[StageId::LINES].m_vShader->getGrShader();
	ppinit.m_shaders[U(ShaderType::FRAGMENT)] =
		m_stages[StageId::LINES].m_fShader->getGrShader();
	m_stages[StageId::LINES].m_ppline = gr->newInstance<Pipeline>(ppinit);

	// Init buffers
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		for(U i = 0; i < m_stages[s].m_vertBuffs.getSize(); ++i)
		{
			m_stages[s].m_vertBuffs[i] = gr->newInstance<Buffer>(
				MAX_VERTS * sizeof(Vertex), BufferUsageBit::VERTEX,
				BufferAccessBit::CLIENT_MAP_WRITE);
		}
	}

	// Init resource groups
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		for(U i = 0; i < m_stages[s].m_rcGroups.getSize(); ++i)
		{
			ResourceGroupInitializer rcinit;
			rcinit.m_vertexBuffers[0].m_buffer = m_stages[s].m_vertBuffs[i];

			if(s == StageId::TEXTURED_TRIANGLES)
			{
				// TODO
			}

			m_stages[s].m_rcGroups[i] = gr->newInstance<ResourceGroup>(rcinit);
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void UiInterfaceImpl::beginRendering(CommandBufferPtr cmdb)
{
	m_cmdb = cmdb;

	// Map buffers
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		BufferPtr buff = m_stages[s].m_vertBuffs[m_timestamp];

		m_vertMappings[s] = static_cast<Vertex*>(
			buff->map(0, sizeof(Vertex) * MAX_VERTS,
			BufferAccessBit::CLIENT_MAP_WRITE));

		m_vertCounts[s] = 0;
	}
}

//==============================================================================
void UiInterfaceImpl::endRendering()
{
	m_cmdb.reset(nullptr);

	for(U s = 0; s < StageId::COUNT; ++s)
	{
		BufferPtr buff = m_stages[s].m_vertBuffs[m_timestamp];
		buff->unmap();
	}

	m_timestamp = (m_timestamp + 1) % MAX_FRAMES_IN_FLIGHT;
}

//==============================================================================
void UiInterfaceImpl::drawLines(
	const SArray<Vec2>& positions, const Color& color)
{
	StageId stageId = StageId::LINES;

	ANKI_ASSERT(m_vertCounts[stageId] + positions.getSize() <= MAX_VERTS);

	m_cmdb->bindPipeline(m_stages[StageId::LINES].m_ppline);
	m_cmdb->bindResourceGroup(m_stages[StageId::LINES].m_rcGroups[m_timestamp]);
	m_cmdb->drawArrays(positions.getSize(), 1, m_vertCounts[stageId]);

	for(const Vec2& pos : positions)
	{
		Vertex v;
		v.m_pos = pos;
		v.m_uv = Vec2(0.0);
		Color c = color * 255.0;
		v.m_color = {{U8(c[0]), U8(c[1]), U8(c[2]), U8(c[3])}};

		m_vertMappings[stageId][m_vertCounts[stageId]] = v;

		++m_vertCounts[stageId];
	}
}

} // end namespace anki
