// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/util/Singleton.h>
#include <anki/util/Array.h>
#include <anki/util/StdTypes.h>
#include <anki/input/KeyCode.h>

namespace anki
{

// Forward
class InputImpl;
class NativeWindow;

enum class InputEvent : U8
{
	WINDOW_FOCUS_LOST,
	WINDOW_FOCUS_GAINED,
	WINDOW_CLOSED,
	COUNT
};

/// Handle the input and other events
/// @note All positions are in NDC space
class Input
{
public:
	Input()
	{
	}

	~Input()
	{
		destroy();
		ANKI_ASSERT(m_impl == nullptr);
		ANKI_ASSERT(m_nativeWindow == nullptr);
	}

	ANKI_USE_RESULT Error create(NativeWindow* nativeWindow)
	{
		reset();
		return init(nativeWindow);
	}

	U getKey(KeyCode i) const
	{
		return m_keys[static_cast<U>(i)];
	}

	U getMouseButton(U32 i) const
	{
		return m_mouseBtns[i];
	}

	const Vec2& getMousePosition() const
	{
		return m_mousePosNdc;
	}

	/// Get the times an event was triggered and resets the counter
	U getEvent(InputEvent eventId) const
	{
		return m_events[static_cast<U>(eventId)];
	}

	/// Reset the keys and mouse buttons
	void reset();

	/// Populate the key and button with the new state
	ANKI_USE_RESULT Error handleEvents();

	/// Move the mouse cursor to a position inside the window. Useful for locking the cursor into a fixed location (eg
	/// in the center of the screen)
	void moveCursor(const Vec2& posNdc);

	/// Hide the mouse cursor
	void hideCursor(Bool hide);

	/// Lock mouse to (0, 0)
	void lockCursor(Bool lock)
	{
		m_lockCurs = lock;
	}

	/// Add a new event
	void addEvent(InputEvent eventId)
	{
		++m_events[static_cast<U>(eventId)];
	}

private:
	InputImpl* m_impl = nullptr;
	NativeWindow* m_nativeWindow = nullptr;

	/// @name Keys and btns
	/// @{

	/// Shows the current key state
	/// - 0 times: unpressed
	/// - 1 times: pressed once
	/// - >1 times: Kept pressed 'n' times continuously
	Array<U32, static_cast<U>(KeyCode::COUNT)> m_keys;

	/// Mouse btns. Supporting 3 btns & wheel. @see keys
	Array<U32, 8> m_mouseBtns;
	/// @}

	Vec2 m_mousePosNdc = Vec2(2.0); ///< The coords are in the NDC space

	Array<U8, static_cast<U>(InputEvent::COUNT)> m_events;

	Bool8 m_lockCurs = false;

	/// Initialize the platform's input system
	ANKI_USE_RESULT Error init(NativeWindow* nativeWindow);

	/// Destroy the platform specific input system
	void destroy();
};

} // end namespace anki
