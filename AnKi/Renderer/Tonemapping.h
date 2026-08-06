// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

ANKI_CVAR2(NumericCVar<F32>, Render, Tonemapping, MinLog2Luminance, -12.0f, -100.0f, 100.0f, "Used in tonemapping calculations");
ANKI_CVAR2(NumericCVar<F32>, Render, Tonemapping, MaxLog2Luminance, 20.0f, -100.0f, 100.0f, "Used in tonemapping calculations");
ANKI_CVAR2(NumericCVar<F32>, Render, Tonemapping, AdaptationRate, 2.1f, 0.0f, 100.0f,
		   "Eye adaptation rate in 1/seconds. Higher adapts faster. Zero freezes the exposure");
ANKI_CVAR2(NumericCVar<F32>, Render, Tonemapping, DarkPixelTrimPercent, 40.0f, 0.0f, 59.0f,
		   "Percentage of the darkest pixels metering ignores. Low blows out dark scenes, high keeps them dark");
ANKI_CVAR2(NumericCVar<F32>, Render, Tonemapping, BrightPixelTrimPercent, 2.0f, 0.0f, 29.0f,
		   "Percentage of the brightest pixels metering ignores. Low lets highlights darken the scene, high brightens it");

// Tonemapping.
class Tonemapping : public RendererObject
{
public:
	Error init();

	void importRenderTargets();

	void populateRenderGraph();

	// See m_exposureAndAvgLuminance1x1
	RenderTargetHandle getExposureAndAvgLuminanceRt() const
	{
		return m_runCtx.m_exposureLuminanceHandle;
	}

	// Final tonemapped texture
	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

private:
	class Histogram
	{
	public:
		static constexpr U32 kBinCount = 256; // Needs to match the HLSL
		RendererShaderProgram m_prog;
		SegregatedListsGpuMemoryPoolAllocation m_histogramBuff;
		U32 m_inputTexMip;
	} m_histogram;

	class
	{
	public:
		RendererShaderProgram m_prog;

		// This is a 1x1 2 component texture where R is the exposure and G the average luminance. It persists across frames because the eye adaptation
		// reads the value of the previous frame.
		RendererTexture m_exposureAndAvgLuminance1x1;
		Bool m_importedOnce = false;
	} m_expAndAvgLum;

	class
	{
	public:
		RendererShaderProgram m_prog;

		RenderTargetDesc m_rtDesc;

		ImageResourcePtr m_lut; // Color grading lookup texture.
	} m_tonemapping;

	class
	{
	public:
		RenderTargetHandle m_exposureLuminanceHandle;
		RenderTargetHandle m_rt;
	} m_runCtx;
};

} // end namespace anki
