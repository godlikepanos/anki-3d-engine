// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Ui/UiImmediateModeBuilder.h>
#include <AnKi/Util/BuddyAllocatorBuilder.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

/// @addtogroup core
/// @{

/// @memberof StatsUi
enum class StatsUiDetail : U8
{
	kDetailed,
	kFpsOnly
};

/// UI for displaying on-screen stats.
class StatsUi : public UiImmediateModeBuilder
{
public:
	StatsUi();

	~StatsUi();

	Error init();

	void build(CanvasPtr ctx) override;

	void setStatsDetail(StatsUiDetail detail)
	{
		m_detail = detail;
	}

private:
	class Value;

	static constexpr U32 kBufferedFrames = 30;

	FontPtr m_font;
	StatsUiDetail m_detail = StatsUiDetail::kDetailed;

	CoreDynamicArray<Value> m_averageValues;
	U32 m_bufferedFrames = 0;

	static void labelTime(Second val, CString name)
	{
		ImGui::Text("%s: %fms", name.cstr(), val * 1000.0);
	}

	static void labelUint(U64 val, CString name)
	{
		ImGui::Text("%s: %lu", name.cstr(), val);
	}

	static void labelBytes(PtrSize val, CString name);
};
/// @}

} // end namespace anki
