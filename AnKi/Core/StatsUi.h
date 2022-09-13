// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
class StatsUiInput
{
public:
#define ANKI_STATS_UI_BEGIN_GROUP(x)
#define ANKI_STATS_UI_VALUE(type, name, text, flags) type m_##name = {};
#include <AnKi/Core/StatsUi.defs.h>
#undef ANKI_STATS_UI_BEGIN_GROUP
#undef ANKI_STATS_UI_VALUE
};

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
	StatsUi(UiManager* ui)
		: UiImmediateModeBuilder(ui)
	{
	}

	~StatsUi();

	Error init();

	void build(CanvasPtr ctx) override;

	void setStats(const StatsUiInput& input, StatsUiDetail detail);

private:
	static constexpr U32 kBufferedFrames = 16;

	enum class ValueFlag : U8
	{
		kNone = 0,
		kAverage = 1 << 0,
		kSeconds = 1 << 1,
		kBytes = 1 << 2,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(ValueFlag)

	class Value
	{
	public:
		union
		{
			U64 m_int = 0;
			F64 m_float;
			F64 m_avg;
		};

		F64 m_rollingAvg = 0.0;

		template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
		void update(T x, ValueFlag flags, Bool flush)
		{
			if(!!(flags & ValueFlag::kAverage))
			{
				setAverage(x, flush);
			}
			else
			{
				m_float = x;
			}
		}

		template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
		void update(T x, ValueFlag flags, Bool flush)
		{
			if(!!(flags & ValueFlag::kAverage))
			{
				setAverage(F64(x), flush);
			}
			else
			{
				m_int = x;
			}
		}

		void setAverage(F64 x, Bool flush)
		{
			m_rollingAvg += x / F64(kBufferedFrames);

			if(flush)
			{
				m_avg = m_rollingAvg;
				m_rollingAvg = 0.0;
			}
		}
	};

#define ANKI_STATS_UI_BEGIN_GROUP(x)
#define ANKI_STATS_UI_VALUE(type, name, text, flags) Value m_##name;
#include <AnKi/Core/StatsUi.defs.h>
#undef ANKI_STATS_UI_BEGIN_GROUP
#undef ANKI_STATS_UI_VALUE

	FontPtr m_font;
	U32 m_bufferedFrames = 0;
	StatsUiDetail m_detail = StatsUiDetail::kDetailed;

	static void labelTime(Second val, CString name)
	{
		ImGui::Text("%s: %fms", name.cstr(), val * 1000.0);
	}

	static void labelUint(U64 val, CString name)
	{
		ImGui::Text("%s: %lu", name.cstr(), val);
	}

	void labelBytes(PtrSize val, CString name) const;
};
/// @}

} // end namespace anki
