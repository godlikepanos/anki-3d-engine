// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>

namespace anki {

class ImageViewerUi
{
public:
	ImageResourcePtr m_image;
	Bool m_open = false;

	ImageViewerUi();

	void drawWindow(UiCanvas& canvas, Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags = 0);

private:
	ShaderProgramResourcePtr m_imageProgram;
	Array<ShaderProgramPtr, 2> m_imageGrPrograms;

	U32 m_crntMip = 0;
	F32 m_zoom = 1.0f;
	F32 m_depth = 0.0f;
	Bool m_pointSampling = true;
	Array<Bool, 4> m_colorChannel = {true, true, true, true};
	F32 m_maxColorValue = 1.0f;

	U32 m_imageUuid = 0;
};

} // end namespace anki
