// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/StatsUiNode.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Core/App.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Renderer/Renderer.h>

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

class StatsUi::Value
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

StatsUi::StatsUi()
{
	if(StatsSet::getSingleton().getCounterCount())
	{
		m_averageValues.resize(StatsSet::getSingleton().getCounterCount());
	}
}

StatsUi::~StatsUi()
{
}

void StatsUi::drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags)
{
	if(!m_open)
	{
		return;
	}

	Bool flush = false;
	if(m_bufferedFrames == kBufferedFrames)
	{
		flush = true;
		m_bufferedFrames = 0;
	}
	++m_bufferedFrames;

	if(ImGui::GetFrameCount() > 1)
	{
		ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
	}

	if(ImGui::Begin("Stats", &m_open, windowFlags))
	{
		StatCategory category = StatCategory::kCount;
		U32 count = 0;
		StatsSet::getSingleton().iterateStats(
			[&](StatCategory c, const Char* name, U64 value, StatFlag flags) {
				if(category != c)
				{
					category = c;
					ImGui::SeparatorText(kStatCategoryTexts[c].cstr());

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
					ImGui::SeparatorText(kStatCategoryTexts[c].cstr());
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
	ImGui::End();
}

StatsUiNode::StatsUiNode(const SceneNodeInitInfo& inf)
	: SceneNode(inf)
{
	UiComponent* uic = newComponent<UiComponent>();
	uic->init(
		[](UiCanvas& canvas, void* ud) {
			static_cast<StatsUiNode*>(ud)->draw(canvas);
		},
		this);

	uic->setEnabled(g_cvarCoreDisplayStats > 0);
	m_statsUi.m_open = true;

	setSerialization(false);
	setUpdateOnPause(true);
}

StatsUiNode::~StatsUiNode()
{
}

void StatsUiNode::update([[maybe_unused]] SceneNodeUpdateInfo& info)
{
	getFirstComponentOfType<UiComponent>().setEnabled(g_cvarCoreDisplayStats > 0);
	m_fpsOnly = g_cvarCoreDisplayStats == 1;
}

void StatsUiNode::draw(UiCanvas& canvas)
{
	if(!m_font)
	{
		m_font = canvas.addFont("EngineAssets/UbuntuMonoRegular.ttf");
	}

	ImGui::PushFont(m_font, 24.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, Vec4(0.0, 0.0, 0.0, 0.3f));

	const Vec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
	const Vec2 viewportPos = ImGui::GetMainViewport()->WorkPos;
	const Vec2 initialPos(viewportPos);
	const Vec2 initialSize(500.0f, viewportSize.y - 20.0f);

	if(!m_fpsOnly)
	{
		m_statsUi.drawWindow(initialPos, initialSize, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
	}
	else
	{
		if(ImGui::GetFrameCount() > 1)
		{
			ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(initialPos, ImGuiCond_FirstUseEver);
		}

		if(ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			const Second maxTime = max(g_svarCpuTotalTime.getValue<F64>(), g_svarRendererGpuTime.getValue<F64>()) / 1000.0;
			const F32 fps = F32(1.0 / maxTime);
			const Bool cpuBound = g_svarCpuTotalTime.getValue<F64>() > g_svarRendererGpuTime.getValue<F64>();
			ImGui::TextColored((cpuBound) ? Vec4(1.0f, 0.5f, 0.5f, 1.0f) : Vec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS %.1f", fps);
		}
		ImGui::End();
	}

	ImGui::PopStyleColor();
	ImGui::PopFont();
}

} // end namespace anki
