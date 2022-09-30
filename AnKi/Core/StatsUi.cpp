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

	return Error::kNone;
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

	StringRaii timestamp(&getMemoryPool());
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

void StatsUi::setStats(const StatsUiInput& input, StatsUiDetail detail)
{
	Bool flush = false;
	if(m_bufferedFrames == kBufferedFrames)
	{
		flush = true;
		m_bufferedFrames = 0;
	}
	++m_bufferedFrames;

#define ANKI_STATS_UI_BEGIN_GROUP(x)
#define ANKI_STATS_UI_VALUE(type, name, text, flags) m_##name.update(input.m_##name, flags, flush);
#include <AnKi/Core/StatsUi.defs.h>
#undef ANKI_STATS_UI_BEGIN_GROUP
#undef ANKI_STATS_UI_VALUE

	m_detail = detail;
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

		auto writeText = [this](const Value& v, const Char* text, ValueFlag flags, Bool isFloat) {
			if(isFloat)
			{
				if(!!(flags & ValueFlag::kSeconds))
				{
					labelTime(v.m_float, text);
				}
				else
				{
					ImGui::Text("%s: %f", text, v.m_float);
				}
			}
			else
			{
				U64 val;
				if(!!(flags & ValueFlag::kAverage))
				{
					val = U64(v.m_avg);
				}
				else
				{
					val = v.m_int;
				}

				if(!!(flags & ValueFlag::kBytes))
				{
					labelBytes(val, text);
				}
				else
				{
					ImGui::Text("%s: %zu", text, val);
				}
			}
		};

		if(m_detail == StatsUiDetail::kDetailed)
		{
#define ANKI_STATS_UI_BEGIN_GROUP(x) \
	ImGui::Text("----"); \
	ImGui::Text(x); \
	ImGui::Text("----");
#define ANKI_STATS_UI_VALUE(type, name, text, flags) \
	writeText(m_##name, text, flags, std::is_floating_point<type>::value);
#include <AnKi/Core/StatsUi.defs.h>
#undef ANKI_STATS_UI_BEGIN_GROUP
#undef ANKI_STATS_UI_VALUE
		}
		else
		{
			const Second maxTime = max(m_cpuFrameTime.m_float, m_gpuFrameTime.m_float);
			const F32 fps = F32(1.0 / maxTime);
			const Bool cpuBound = m_cpuFrameTime.m_float > m_gpuFrameTime.m_float;
			ImGui::TextColored((cpuBound) ? Vec4(1.0f, 0.5f, 0.5f, 1.0f) : Vec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS %.1f",
							   fps);
		}
	}

	ImGui::End();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	canvas->popFont();
}

} // end namespace anki
