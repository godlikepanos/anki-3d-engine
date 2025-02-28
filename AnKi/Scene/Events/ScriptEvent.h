// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Script/ScriptEnvironment.h>

namespace anki {

/// @addtogroup scene
/// @{

/// A generic event that uses scripts. The script should contain something like this:
/// @code
/// function update(event, prevTime, crntTime)
/// 	-- Do something
/// end
///
/// function onKilled(event, prevTime, crntTime)
/// 	-- Do something
/// end
/// @endcode
class ScriptEvent : public Event
{
public:
	ScriptEvent(Second startTime, Second duration, CString script);

	~ScriptEvent();

	void update(Second prevUpdateTime, Second crntTime) override;

	void onKilled(Second prevUpdateTime, Second crntTime) override;

private:
	ScriptResourcePtr m_scriptRsrc;
	SceneString m_script;
	ScriptEnvironment m_env;
};
/// @}

} // end namespace anki
