// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Input/KeyCode.h>

namespace anki {

// Forward
class InputImpl;
class NativeWindow;

enum class InputEvent : U8
{
	kWindowFocusLost,
	kWindowFocusGained,
	kWindowClosed,
	kCount
};

/// Handle the input and other events
/// @note All positions are in NDC space
class Input
{
	ANKI_FRIEND_CALL_CONSTRUCTOR_AND_DESTRUCTOR

public:
	static Error newInstance(AllocAlignedCallback allocCallback, void* allocCallbackUserData,
							 NativeWindow* nativeWindow, Input*& input);

	static void deleteInstance(Input* input);

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
	U32 getEvent(InputEvent eventId) const
	{
		return m_events[eventId];
	}

	/// Reset the keys and mouse buttons
	void reset()
	{
		zeroMemory(m_keys);
		zeroMemory(m_mouseBtns);
		m_mousePosNdc = Vec2(-1.0f);
		m_mousePosWin = UVec2(0u);
		zeroMemory(m_events);
		zeroMemory(m_textInput);
		zeroMemory(m_touchPointers);
		zeroMemory(m_touchPointerPosNdc);
		zeroMemory(m_touchPointerPosWin);
	}

	/// Populate the key and button with the new state
	Error handleEvents();

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
		++m_events[eventId];
	}

	template<typename TFunc>
	void iteratePressedKeys(TFunc func) const
	{
		for(const KeyCode i : EnumIterable<KeyCode>())
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

	U32 getTouchPointer(TouchPointer p) const
	{
		return m_touchPointers[p];
	}

	Vec2 getTouchPointerNdcPosition(TouchPointer p) const
	{
		return m_touchPointerPosNdc[p];
	}

	UVec2 getTouchPointerWindowPosition(TouchPointer p) const
	{
		return m_touchPointerPosWin[p];
	}

	Bool hasTouchDevice() const;

protected:
	NativeWindow* m_nativeWindow = nullptr;
	HeapMemoryPool m_pool;

	/// Shows the current key state
	/// - 0 times: unpressed
	/// - 1 times: pressed once
	/// - >1 times: Kept pressed 'n' times continuously
	Array<U32, U(KeyCode::kCount)> m_keys;

	/// Mouse btns. Supporting 3 btns & wheel. @see keys
	Array<U32, U(MouseButton::kCount)> m_mouseBtns;
	Vec2 m_mousePosNdc;
	UVec2 m_mousePosWin;

	Array<U32, U(TouchPointer::kCount)> m_touchPointers;
	Array<Vec2, U(TouchPointer::kCount)> m_touchPointerPosNdc;
	Array<UVec2, U(TouchPointer::kCount)> m_touchPointerPosWin;

	Array<U8, U(InputEvent::kCount)> m_events;

	/// The keybord input as ascii.
	Array<char, U(KeyCode::kCount)> m_textInput;

	Bool m_lockCurs = false;

	Input()
	{
		reset();
	}

	~Input()
	{
	}
};

} // end namespace anki
