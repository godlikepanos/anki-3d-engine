// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/AnKi.h>

class SampleApp : public anki::App
{
public:
	anki::Error init(int argc, char** argv, anki::CString sampleName);
	anki::Error userMainLoop(anki::Bool& quit, anki::Second elapsedTime) override;

	virtual anki::Error sampleExtraInit() = 0;
};