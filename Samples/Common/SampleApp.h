// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>

namespace anki {

class SampleApp : public App
{
public:
	using App::App;

	Error init(int argc, char** argv, CString sampleName);
	Error userMainLoop(Bool& quit, Second elapsedTime) override;

	virtual Error sampleExtraInit() = 0;
};

} // end namespace anki
