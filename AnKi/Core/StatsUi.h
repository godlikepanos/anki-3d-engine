// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Ui/UiImmediateModeBuilder.h>

namespace anki {

/// @addtogroup core
/// @{

/// XXX
class StatsUi : public UiImmediateModeBuilder
{
public:
	StatsUi(UiManager* ui)
		: UiImmediateModeBuilder(ui)
	{
	}

	~StatsUi();

	ANKI_USE_RESULT Error init();

	void build(CanvasPtr ctx) override;

	void setFrameTime(Second v)
	{
		m_frameTime.set(v);
	}

	void setRenderTime(Second v)
	{
		m_renderTime.set(v);
	}

	void setSceneUpdateTime(Second v)
	{
		m_sceneUpdateTime.set(v);
	}

	void setVisibilityTestsTime(Second v)
	{
		m_visTestsTime.set(v);
	}

	void setPhysicsTime(Second v)
	{
		m_physicsTime.set(v);
	}

	void setGpuTime(Second v)
	{
		m_gpuTime.set(v);
	}

	void setGpuActiveCycles(U64 v)
	{
		m_gpuActive.set(v);
	}

	void setGpuReadBandwidth(PtrSize v)
	{
		m_gpuReadBandwidth.set(v);
	}

	void setGpuWriteBandwidth(PtrSize v)
	{
		m_gpuWriteBandwidth.set(v);
	}

	void setAllocatedCpuMemory(PtrSize v)
	{
		m_allocatedCpuMem = v;
	}

	void setCpuAllocationCount(U64 v)
	{
		m_allocCount = v;
	}

	void setCpuFreeCount(U64 v)
	{
		m_freeCount = v;
	}

	void setVkCpuMemory(PtrSize v)
	{
		m_vkCpuMem = v;
	}

	void setVkGpuMemory(PtrSize v)
	{
		m_vkGpuMem = v;
	}

	void setVkCommandBufferCount(U32 v)
	{
		m_vkCmdbCount = v;
	}

	void setDrawableCount(U64 v)
	{
		m_drawableCount = v;
	}

private:
	static constexpr U32 BUFFERED_FRAMES = 16;

	template<typename T>
	class BufferedValue
	{
	public:
		void set(T x)
		{
			m_rollongAvg += x / T(BUFFERED_FRAMES);
		}

		T get(Bool flush)
		{
			if(flush)
			{
				m_avg = m_rollongAvg;
				m_rollongAvg = T(0);
			}

			return m_avg;
		}

	private:
		T m_rollongAvg = T(0);
		T m_avg = T(0);
	};

	FontPtr m_font;
	U32 m_bufferedFrames = 0;

	// CPU
	BufferedValue<Second> m_frameTime;
	BufferedValue<Second> m_renderTime;
	BufferedValue<Second> m_sceneUpdateTime;
	BufferedValue<Second> m_visTestsTime;
	BufferedValue<Second> m_physicsTime;

	// GPU
	BufferedValue<Second> m_gpuTime;
	BufferedValue<U64> m_gpuActive;
	BufferedValue<PtrSize> m_gpuReadBandwidth;
	BufferedValue<PtrSize> m_gpuWriteBandwidth;

	// Memory
	PtrSize m_allocatedCpuMem = 0;
	U64 m_allocCount = 0;
	U64 m_freeCount = 0;
	PtrSize m_vkCpuMem = 0;
	PtrSize m_vkGpuMem = 0;

	// Vulkan
	U32 m_vkCmdbCount = 0;

	// Other
	PtrSize m_drawableCount = 0;

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
