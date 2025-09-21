// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Ui/Canvas.h>

namespace anki {

/// @addtogroup scene
/// @{

/// A node that draws a UI with stats.
class StatsUiNode : public SceneNode
{
public:
	StatsUiNode(CString name);

	~StatsUiNode();

	void setFpsOnly(Bool fpsOnly)
	{
		m_fpsOnly = fpsOnly;
	}

private:
	class Value;

	static constexpr U32 kBufferedFrames = 30;

	FontPtr m_font;
	Bool m_fpsOnly = false;

	SceneDynamicArray<Value> m_averageValues;
	U32 m_bufferedFrames = 0;

	void draw(Canvas& canvas);
};
/// @}

} // end namespace anki
