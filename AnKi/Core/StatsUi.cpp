// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/StatsUi.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/Canvas.h>

namespace anki {

StatsUi::~StatsUi()
{
}

Error StatsUi::init()
{
	ANKI_CHECK(m_manager->newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf", Array<U32, 1>{24}));

	return Error::NONE;
}

void StatsUi::labelBytes(PtrSize val, CString name) const
{
	PtrSize gb, mb, kb, b;

	gb = val / 1_GB;
	val -= gb * 1_GB;

	mb = val / 1_MB;
	val -= mb * 1_MB;

	kb = val / 1_KB;
	val -= kb * 1_KB;

	b = val;

	StringAuto timestamp(getAllocator());
	if(gb)
	{
		timestamp.sprintf("%s: %u,%04u,%04u,%04u", name.cstr(), gb, mb, kb, b);
	}
	else if(mb)
	{
		timestamp.sprintf("%s: %u,%04u,%04u", name.cstr(), mb, kb, b);
	}
	else if(kb)
	{
		timestamp.sprintf("%s: %u,%04u", name.cstr(), kb, b);
	}
	else
	{
		timestamp.sprintf("%s: %u", name.cstr(), b);
	}
	ImGui::TextUnformatted(timestamp.cstr());
}

void StatsUi::build(CanvasPtr canvas)
{
	// Misc
	++m_bufferedFrames;
	Bool flush = false;
	if(m_bufferedFrames == BUFFERED_FRAMES)
	{
		flush = true;
		m_bufferedFrames = 0;
	}

	// Start drawing the UI
	canvas->pushFont(m_font, 24);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.3f;

	if(ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(Vec2(5.0f, 5.0f));
		ImGui::SetWindowSize(Vec2(230.0f, 450.0f));

		ImGui::Text("CPU Time:");
		labelTime(m_frameTime.get(flush), "Total frame");
		labelTime(m_renderTime.get(flush), "Renderer");
		labelTime(m_sceneUpdateTime.get(flush), "Scene update");
		labelTime(m_visTestsTime.get(flush), "Visibility");
		labelTime(m_physicsTime.get(flush), "Physics");

		ImGui::Text("----");
		ImGui::Text("GPU:");
		labelTime(m_gpuTime.get(flush), "Total frame");
		const U64 gpuActive = m_gpuActive.get(flush);
		if(gpuActive)
		{
			ImGui::Text("%s: %luK cycles", "Active Cycles", gpuActive / 1000);
			labelBytes(m_gpuReadBandwidth.get(flush), "Read bandwidth");
			labelBytes(m_gpuWriteBandwidth.get(flush), "Write bandwidth");
		}

		ImGui::Text("----");
		ImGui::Text("Memory:");
		labelBytes(m_allocatedCpuMem, "Total CPU");
		labelUint(m_allocCount, "Total allocations");
		labelUint(m_freeCount, "Total frees");
		labelBytes(m_vkCpuMem, "Vulkan CPU");
		labelBytes(m_vkGpuMem, "Vulkan GPU");
		labelBytes(m_globalVertexPoolStats.m_userAllocatedSize, "Vertex/Index GPU memory");
		labelBytes(m_globalVertexPoolStats.m_realAllocatedSize, "Actual Vertex/Index GPU memory");

		ImGui::Text("----");
		ImGui::Text("Vulkan:");
		labelUint(m_vkCmdbCount, "Cmd buffers");

		ImGui::Text("----");
		ImGui::Text("Other:");
		labelUint(m_drawableCount, "Drawbles");
	}

	ImGui::End();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	canvas->popFont();
}

} // end namespace anki
