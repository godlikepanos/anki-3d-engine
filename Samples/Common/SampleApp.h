// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>

class SampleApp : public anki::App
{
public:
	anki::ConfigSet m_config;

	anki::CanvasPtr m_canvas;

	anki::Second m_timesOfLastTouchEvent = 0.0f;
	static constexpr anki::Second kIdleTime = 12.0;
	class anki::AnimationEvent* m_cameraAnimationEvent = nullptr;

	anki::Error init(int argc, char** argv, anki::CString sampleName);
	anki::Error userMainLoop(anki::Bool& quit, anki::Second elapsedTime) override;

	virtual anki::Error sampleExtraInit() = 0;
};
