// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/StatsUiNode.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Core/App.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Font.h>

namespace anki {

static void labelBytes(PtrSize val, CString name)
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

class StatsUiNode::Value
{
public:
	F64 m_avg = 0.0;
	F64 m_rollingAvg = 0.0;

	void update(F64 x, Bool flush)
	{
		m_rollingAvg += x / F64(kBufferedFrames);

		if(flush)
		{
			m_avg = m_rollingAvg;
			m_rollingAvg = 0.0;
		}
	}
};

StatsUiNode::StatsUiNode(CString name)
	: SceneNode(name)
{
	UiComponent* uic = newComponent<UiComponent>();
	uic->init(
		[](CanvasPtr& canvas, void* ud) {
			static_cast<StatsUiNode*>(ud)->draw(canvas);
		},
		this);

	if(StatsSet::getSingleton().getCounterCount())
	{
		m_averageValues.resize(StatsSet::getSingleton().getCounterCount());
	}
}

StatsUiNode::~StatsUiNode()
{
}

Error StatsUiNode::init()
{
	ANKI_CHECK(UiManager::getSingleton().newInstance(m_font, "EngineAssets/UbuntuMonoRegular.ttf", Array<U32, 1>{24}));
	return Error::kNone;
}

void StatsUiNode::draw(CanvasPtr& canvas)
{
	Bool flush = false;
	if(m_bufferedFrames == kBufferedFrames)
	{
		flush = true;
		m_bufferedFrames = 0;
	}
	++m_bufferedFrames;

	canvas->pushFont(m_font, 24);

	const Vec4 oldWindowColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 0.3f;

	if(ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(Vec2(5.0f, 5.0f));
		ImGui::SetWindowSize(Vec2(230.0f, 450.0f));

		if(!m_fpsOnly)
		{
			StatCategory category = StatCategory::kCount;
			U32 count = 0;
			StatsSet::getSingleton().iterateStats(
				[&](StatCategory c, const Char* name, U64 value, StatFlag flags) {
					if(category != c)
					{
						category = c;
						ImGui::Text("-- %s --", kStatCategoryTexts[c].cstr());

						// Hack
						if(c == StatCategory::kMisc)
						{
							ImGui::Text("Frame: %zu", GlobalFrameIndex::getSingleton().m_value);
						}
					}

					if(!!(flags & StatFlag::kBytes))
					{
						labelBytes(value, name);
					}
					else
					{
						ImGui::Text("%s: %zu", name, value);
					}

					++count;
				},
				[&](StatCategory c, const Char* name, F64 value, StatFlag flags) {
					if(category != c)
					{
						category = c;
						ImGui::Text("-- %s --", kStatCategoryTexts[c].cstr());
					}

					if(!!(flags & StatFlag::kShowAverage))
					{
						m_averageValues[count].update(value, flush);
						value = m_averageValues[count].m_avg;
					}

					ImGui::Text("%s: %f", name, value);
					++count;
				});
		}
		else
		{
			const Second maxTime = max(g_cpuTotalTimeStatVar.getValue<F64>(), g_rendererGpuTimeStatVar.getValue<F64>()) / 1000.0;
			const F32 fps = F32(1.0 / maxTime);
			const Bool cpuBound = g_cpuTotalTimeStatVar.getValue<F64>() > g_rendererGpuTimeStatVar.getValue<F64>();
			ImGui::TextColored((cpuBound) ? Vec4(1.0f, 0.5f, 0.5f, 1.0f) : Vec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS %.1f", fps);
		}
	}

	ImGui::End();
	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = oldWindowColor;

	canvas->popFont();
}

} // end namespace anki
