// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiInterface.h>
#include <anki/input/KeyCode.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// UI canvas. It's a container of widgets.
class Canvas
{
public:
	Canvas(UiInterface* interface);

	~Canvas();

	/// Create a new widget.
	template<typename TWidget>
	TWidget* newWidget()
	{
		TWidget* out = getAllocator().newInstance<TWidget>(this);
		return out;
	}

	/// @name Input injection methods.
	/// @{
	void injectMouseMove(const UVec2& pos);

	void injectKeyDown(KeyCode key);

	void injectKeyUp(KeyCode key);

	void injectText(const CString& text);

	void injectMouseButtonDown(MouseButton button);
	/// @}

	/// Update the widgets.
	void update(F64 dt);

	/// Paint the widgets (if needed). Call after update.
	void paint();

	void setSize(const UVec2& size);

	const UVec2& getSize() const
	{
		return m_size;
	}

	Bool getDebugDrawEnabled() const
	{
		return m_debugDrawEnabled;
	}

	void setDebugDrawEnabled()
	{
		m_debugDrawEnabled = true;
	}

#ifdef ANKI_BUILD
	UiAllocator getAllocator() const
	{
		return m_interface->getAllocator();
	}

	Atomic<I32>& getMarkedForDeletionCount()
	{
		return m_markedForDeletionCount;
	}
#endif

private:
	WeakPtr<UiInterface> m_interface;
	Atomic<I32> m_markedForDeletionCount = {0};
	WeakPtr<Widget> m_rootWidget;

	UVec2 m_size; ///< Virtual size of canvas.
	Bool8 m_debugDrawEnabled = false;
};
/// @}

} // end namespace anki
