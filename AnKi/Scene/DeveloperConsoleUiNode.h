// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Ui/UiCanvas.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

/// @addtogroup scene
/// @{

/// A node that draws the developer console.
class DeveloperConsoleUiNode : public SceneNode
{
public:
	DeveloperConsoleUiNode(CString name);

	~DeveloperConsoleUiNode();

	void toggleConsole();

	Bool isConsoleEnabled() const
	{
		return m_enabled;
	}

private:
	static constexpr U kMaxLogItems = 64;

	class LogItem : public IntrusiveListEnabled<LogItem>
	{
	public:
		const Char* m_file;
		const Char* m_func;
		const Char* m_subsystem;
		SceneString m_threadName;
		SceneString m_msg;
		I32 m_line;
		LoggerMessageType m_type;
	};

	ImFont* m_font = nullptr;
	IntrusiveList<LogItem> m_logItems;
	U32 m_logItemCount = 0;
	Array<Char, 256> m_inputText = {};

	Atomic<U32> m_logItemsTimestamp = {1};
	U32 m_logItemsTimestampConsumed = 0;

	ScriptEnvironment m_scriptEnv;

	Bool m_enabled = false;

	void newLogItem(const LoggerMessageInfo& inf);

	static void loggerCallback(void* userData, const LoggerMessageInfo& info)
	{
		static_cast<DeveloperConsoleUiNode*>(userData)->newLogItem(info);
	}

	void draw(UiCanvas& canvas);
};
/// @}

} // end namespace anki
