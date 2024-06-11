// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/MipmapGenerator.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error MipmapGenerator::init()
{
	ANKI_CHECK(loadShaderProgram("ShaderBinaries/MipmapGenerator.ankiprogbin", m_genMipsProg, m_genMipsGrProg));

	return Error::kNone;
}

void MipmapGenerator::populateRenderGraph(const MipmapGeneratorTargetArguments& target, RenderGraphBuilder& rgraph, CString passesName)
{
	for(U32 readMip = 0; readMip < target.m_mipmapCount - 1u; ++readMip)
	{
		const U8 faceCount = (target.m_isCubeTexture) ? 6 : 1;
		for(U32 face = 0; face < faceCount; ++face)
		{
			for(U32 layer = 0; layer < target.m_layerCount; ++layer)
			{
				GraphicsRenderPass& rpass =
					rgraph.newGraphicsRenderPass(generateTempPassName("%s: mip #%u face #%u layer #%u", passesName.cstr(), readMip, face, layer));

				rpass.newTextureDependency(target.m_handle, TextureUsageBit::kSampledFragment, TextureSubresourceDesc::surface(readMip, face, layer));
				rpass.newTextureDependency(target.m_handle, TextureUsageBit::kFramebufferWrite,
										   TextureSubresourceDesc::surface(readMip + 1, face, layer));

				GraphicsRenderPassTargetDesc rtInfo;
				rtInfo.m_handle = target.m_handle;
				rtInfo.m_subresource = TextureSubresourceDesc::surface(readMip + 1, face, layer);
				rpass.setRenderpassInfo({rtInfo});

				rpass.setWork([this, rt = target.m_handle, readMip, face, layer,
							   viewport = target.m_targetSize >> (readMip + 1)](RenderPassWorkContext& rgraphCtx) {
					ANKI_TRACE_SCOPED_EVENT(MipmapGenerator);

					CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

					cmdb.bindShaderProgram(m_genMipsGrProg.get());
					cmdb.setViewport(0, 0, viewport.x(), viewport.y());

					rgraphCtx.bindTexture(ANKI_REG(t0), rt, TextureSubresourceDesc::surface(readMip, face, layer));
					cmdb.bindSampler(ANKI_REG(s0), getRenderer().getSamplers().m_trilinearClamp.get());

					drawQuad(cmdb);
				});
			}
		}
	}
}

} // end namespace anki
