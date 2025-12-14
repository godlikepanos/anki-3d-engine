// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/AnKi.h>

namespace anki {

class SampleApp : public App
{
public:
	U32 m_argc = 0;
	Char** m_argv = nullptr;

	SampleApp(U32 argc, Char** argv, CString appName)
		: App(appName)
		, m_argc(argc)
		, m_argv(argv)
	{
	}

	Error userPreInit() override;

	Error userPostInit() override
	{
		return sampleExtraInit();
	}

	Error userMainLoop(Bool& quit, Second elapsedTime) override;

	virtual Error sampleExtraInit() = 0;
};

} // end namespace anki
