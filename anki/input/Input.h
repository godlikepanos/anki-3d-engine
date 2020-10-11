// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>
#include <anki/util/Singleton.h>
#include <anki/util/Array.h>
#include <anki/util/String.h>
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

	ANKI_USE_RESULT Error init(NativeWindow* nativeWindow)
	{
		reset();
		return initInternal(nativeWindow);
	}

	U32 getKey(KeyCode i) const
	{
		return m_keys[i];
	}

	U32 getMouseButton(MouseButton i) const
	{
		return m_mouseBtns[i];
	}

	const Vec2& getMousePosition() const
	{
		return m_mousePosNdc;
	}

	const UVec2& getWindowMousePosition() const
	{
		return m_mousePosWin;
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

	template<typename TFunc>
	void iteratePressedKeys(TFunc func) const
	{
		for(KeyCode i = KeyCode::FIRST; i < KeyCode::COUNT; ++i)
		{
			if(m_keys[i] > 0)
			{
				func(i, m_keys[i]);
			}
		}
	}

	/// Get some easy to digest input from the keyboard.
	CString getTextInput() const
	{
		return &m_textInput[0];
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
	Array<U32, U(MouseButton::COUNT)> m_mouseBtns;
	/// @}

	Vec2 m_mousePosNdc = Vec2(2.0); ///< The coords are in the NDC space
	UVec2 m_mousePosWin = UVec2(0u);

	Array<U8, static_cast<U>(InputEvent::COUNT)> m_events;

	/// The keybord input as ascii.
	Array<char, static_cast<U>(KeyCode::COUNT)> m_textInput;

	Bool m_lockCurs = false;

	/// Initialize the platform's input system
	ANKI_USE_RESULT Error initInternal(NativeWindow* nativeWindow);

	/// Destroy the platform specific input system
	void destroy();
};

} // end namespace anki
