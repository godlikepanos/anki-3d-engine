// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Editor/StreamingControlsUi.h>
#include <AnKi/Resource/StreamingImageResourceManager.h>

namespace anki {

void StreamingControlsUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Streaming Controls", &m_open, windowFlags))
	{
		// CVars
		{
			ImGui::SeparatorText("CVars");

			// Disabled. The descriptor buffer is allocated once on init
			ImGui::BeginDisabled();
			I32 maxImageDescriptors = I32(g_cvarRsrcMaxImageDescriptors);
			ImGui::SliderInt(g_cvarRsrcMaxImageDescriptors.getName().cstr(), &maxImageDescriptors, I32(g_cvarRsrcMaxImageDescriptors.getMin()),
							 I32(g_cvarRsrcMaxImageDescriptors.getMax()), "%d", ImGuiSliderFlags_AlwaysClamp);
			ImGui::EndDisabled();
			if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip("%s", g_cvarRsrcMaxImageDescriptors.getDescription().cstr());
			}

			F32 maxTexMemLoadFactor = g_cvarRsrcMaxTextureMemoryLoadFactor;
			if(ImGui::SliderFloat(g_cvarRsrcMaxTextureMemoryLoadFactor.getName().cstr(), &maxTexMemLoadFactor,
								  g_cvarRsrcMaxTextureMemoryLoadFactor.getMin(), g_cvarRsrcMaxTextureMemoryLoadFactor.getMax(), "%.2f",
								  ImGuiSliderFlags_AlwaysClamp))
			{
				g_cvarRsrcMaxTextureMemoryLoadFactor = maxTexMemLoadFactor;
			}
			ImGui::SetItemTooltip("%s", g_cvarRsrcMaxTextureMemoryLoadFactor.getDescription().cstr());

			const F64 texMemLimit = F64(g_cvarGpuMemTextureMemoryPoolChunkSize * g_cvarGpuMemTextureMemoryPoolMaxChunks) * maxTexMemLoadFactor;
			ImGui::Text("(%.2f is %.2f MB of texture memory)", maxTexMemLoadFactor, texMemLimit / F64(1_MB));

			I32 maxMipUploadsPerFrame = I32(g_cvarRsrcMaxMipmapUploadsPerFrame);
			if(ImGui::SliderInt(g_cvarRsrcMaxMipmapUploadsPerFrame.getName().cstr(), &maxMipUploadsPerFrame,
								g_cvarRsrcMaxMipmapUploadsPerFrame.getMin(), g_cvarRsrcMaxMipmapUploadsPerFrame.getMax(), "%d",
								ImGuiSliderFlags_AlwaysClamp))
			{
				g_cvarRsrcMaxMipmapUploadsPerFrame = U32(maxMipUploadsPerFrame);
			}
			ImGui::SetItemTooltip("%s", g_cvarRsrcMaxMipmapUploadsPerFrame.getDescription().cstr());

			I32 framesUntilEviction = I32(g_cvarRsrcFramesUntilEviction);
			if(ImGui::SliderInt(g_cvarRsrcFramesUntilEviction.getName().cstr(), &framesUntilEviction, g_cvarRsrcFramesUntilEviction.getMin(),
								g_cvarRsrcFramesUntilEviction.getMax(), "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				g_cvarRsrcFramesUntilEviction = U32(framesUntilEviction);
			}
			ImGui::SetItemTooltip("%s", g_cvarRsrcFramesUntilEviction.getDescription().cstr());
		}

		// Stats
		{
			ImGui::SeparatorText("Stats");

			ImGui::Text("%s: %zu", g_svarRsrcStreamingRequestCount.getDescription().cstr(), g_svarRsrcStreamingRequestCount.getPreviousValue<U64>());
			ImGui::Text("%s: %zu", g_svarRsrcStreamingMipsUploaded.getDescription().cstr(), g_svarRsrcStreamingMipsUploaded.getPreviousValue<U64>());
			ImGui::Text("%s: %zu", g_svarRsrcTotalStreamingMipsUploaded.getDescription().cstr(),
						g_svarRsrcTotalStreamingMipsUploaded.getPreviousValue<U64>());
			ImGui::Text("%s: %zu", g_svarRsrcStreamingMipsEvicted.getDescription().cstr(), g_svarRsrcStreamingMipsEvicted.getPreviousValue<U64>());
			ImGui::Text("%s: %zu", g_svarRsrcTotalStreamingMipsEvicted.getDescription().cstr(),
						g_svarRsrcTotalStreamingMipsEvicted.getPreviousValue<U64>());

			ImGui::Text("%s: %.2f MB", g_svarGpuMemTextureMemoryPoolCapacity.getDescription().cstr(),
						F64(g_svarGpuMemTextureMemoryPoolCapacity.getPreviousValue<U64>()) / F64(1_MB));
			ImGui::Text("%s: %.2f MB", g_svarGpuMemTextureMemoryPoolUsedMemory.getDescription().cstr(),
						F64(g_svarGpuMemTextureMemoryPoolUsedMemory.getPreviousValue<U64>()) / F64(1_MB));
		}

		if(ImGui::Button("Force Evict All"))
		{
			StreamingImageResourceManager::getSingleton().forceEvictAll();
		}
	}
	ImGui::End();
}

} // end namespace anki
