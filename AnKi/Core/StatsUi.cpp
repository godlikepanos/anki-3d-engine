// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/StatsUi.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/Canvas.h>

namespace anki {

extern StatCounter g_cpuTotalTime;
extern StatCounter g_rendererGpuTime;

StatsUi::~StatsUi()
{
}

Error StatsUi::init()
{
	ANKI_CHECK(UiManager::getSingleton().newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf", Array<U32, 1>{24}));

	return Error::kNone;
}

void StatsUi::labelBytes(PtrSize val, CString name)
{
	PtrSize gb, mb, kb, b;

	gb = val / 1_GB;
	val -= gb * 1_GB;

	mb = val / 1_MB;
	val -= mb * 1_MB;

	kb = val / 1_KB;
	val -= kb * 1_KB;

	b = val;

	UiString timestamp;
	if(gb)
	{
		timestamp.sprintf("%s: %zu,%04zu,%04zu,%04zu", name.cstr(), gb, mb, kb, b);
	}
	else if(mb)
	{
		timestamp.sprintf("%s: %zu,%04zu,%04zu", name.cstr(), mb, kb, b);
	}
	else if(kb)
	{
		timestamp.sprintf("%s: %zu,%04zu", name.cstr(), kb, b);
	}
	else
	{
		timestamp.sprintf("%s: %zu", name.cstr(), b);
	}
	ImGui::TextUnformatted(timestamp.cstr());
}

void StatsUi::build(CanvasPtr canvas)
{
	canvas->pushFont(m_font, 24);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.3f;

	if(ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(Vec2(5.0f, 5.0f));
		ImGui::SetWindowSize(Vec2(230.0f, 450.0f));

		if(m_detail == StatsUiDetail::kDetailed)
		{
			StatCategory category = StatCategory::kCount;
			StatsSet::getSingleton().iterateStats(
				[&](StatCategory c, const Char* name, U64 value, StatFlag flags) {
					if(category != c)
					{
						category = c;
						ImGui::Text("-- %s --", kStatCategoryTexts[c].cstr());
					}

					if(!!(flags & StatFlag::kBytes))
					{
						labelBytes(value, name);
					}
					else
					{
						ImGui::Text("%s: %zu", name, value);
					}
				},
				[&](StatCategory c, const Char* name, F64 value, StatFlag) {
					if(category != c)
					{
						category = c;
						ImGui::Text("-- %s --", kStatCategoryTexts[c].cstr());
					}

					ImGui::Text("%s: %f", name, value);
				});
		}
		else
		{
			const Second maxTime = max(g_cpuTotalTime.getValue<F64>(), g_rendererGpuTime.getValue<F64>()) / 1000.0;
			const F32 fps = F32(1.0 / maxTime);
			const Bool cpuBound = g_cpuTotalTime.getValue<F64>() > g_rendererGpuTime.getValue<F64>();
			ImGui::TextColored((cpuBound) ? Vec4(1.0f, 0.5f, 0.5f, 1.0f) : Vec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS %.1f", fps);
		}
	}

	ImGui::End();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	canvas->popFont();
}

} // end namespace anki
