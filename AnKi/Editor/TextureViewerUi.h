// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Editor/EditorCommon.h>

namespace anki {

class TextureViewerUi : public EditorUiBase
{
public:
	TextureViewerUi();

	void open(Texture* tex);

	void open(ImageResource* img);

	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags = 0);

private:
	TexturePtr m_tex;
	ImageResourcePtr m_img;

	ShaderProgramResourcePtr m_imageProgram;
	Array<ShaderProgramPtr, 2> m_imageGrPrograms;

	U32 m_crntMip = 0;
	F32 m_zoom = 1.0f;
	F32 m_depth = 0.0f;
	Bool m_pointSampling = true;
	Array<Bool, 4> m_colorChannel = {true, true, true, true};
	F32 m_maxColorValue = 1.0f;

	U32 m_texUuid = 0;

	Bool m_open = false;
};

} // end namespace anki
