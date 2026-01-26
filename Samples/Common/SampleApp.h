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
	SampleApp(U32 argc, Char** argv, CString appName)
		: App(appName, argc, argv)
	{
	}

	Error userPreInit() override;

	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

} // end namespace anki
