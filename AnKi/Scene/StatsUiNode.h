// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Ui/UiCanvas.h>

namespace anki {

class StatsUi
{
public:
	Bool m_open = false;

	StatsUi();

	~StatsUi();

	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags);

private:
	static constexpr U32 kBufferedFrames = 30;

	class Value;

	SceneDynamicArray<Value> m_averageValues;
	U32 m_bufferedFrames = 0;
};

// A node that draws a UI with stats.
class StatsUiNode : public SceneNode
{
public:
	StatsUiNode(const SceneNodeInitInfo& inf);

	~StatsUiNode();

	void setFpsOnly(Bool fpsOnly)
	{
		m_fpsOnly = fpsOnly;
	}

private:
	ImFont* m_font = nullptr;
	Bool m_fpsOnly = false;

	StatsUi m_statsUi;

	void update(SceneNodeUpdateInfo& info) override;

	void draw(UiCanvas& canvas);
};

} // end namespace anki
