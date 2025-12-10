// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Window/KeyCode.h>

namespace anki {

enum class InputEvent : U8
{
	kWindowFocusLost,
	kWindowFocusGained,
	kWindowClosed,
	kCount
};

enum class MouseCursor : U8
{
	kArrow,
	kTextInput, // When hovering over InputText, etc.
	kResizeAll,
	kResizeNS, // When hovering over a horizontal border
	kResizeEW, // When hovering over a vertical border or a column
	kResizeNESW, // When hovering over the bottom-left corner of a window
	kResizeNWSE, // When hovering over the bottom-right corner of a window
	kHand,
	kWait, // When waiting for something to process/load.
	kProgress, // When waiting for something to process/load, but application is still interactive.
	kNotAllowed,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MouseCursor)

/// Handle the input and other events
/// @note All positions are in NDC space
class Input : public MakeSingletonPtr<Input>
{
	template<typename>
	friend class MakeSingletonPtr;

public:
	Error init();

	/// Shows the current key state
	/// 0: Key resting
	/// 1: Ley pressed once
	/// >1: Kept pressed 'n' times continuously
	/// <0: Key up
	I32 getKey(KeyCode i) const
	{
		return m_keys[i];
	}

	/// See getKey()
	I32 getMouseButton(MouseButton i) const
	{
		return m_mouseBtns[i];
	}

	const Vec2& getMousePositionNdc() const
	{
		return m_mousePosNdc;
	}

	const Vec2& getMousePreviousPositionNdc() const
	{
		return m_prevMousePosNdc;
	}

	Vec2 getMouseMoveDeltaNdc() const
	{
		return m_mousePosNdc - m_prevMousePosNdc;
	}

	/// Move the mouse cursor to a position inside the window. Useful for locking the cursor into a fixed location (eg in the center of the screen)
	void moveMouseNdc(const Vec2& posNdc);

	/// Lock mouse to (0, 0)
	void lockMouseWindowCenter(Bool lock)
	{
		m_lockCurs = lock;
	}

	/// Hide the mouse cursor
	void hideCursor(Bool hide);

	void setMouseCursor(MouseCursor cursor);

	/// See getKey()
	I32 getTouchPointer(TouchPointer p) const
	{
		return m_touchPointers[p];
	}

	Vec2 getTouchPointerNdcPosition(TouchPointer p) const
	{
		return m_touchPointerPosNdc[p];
	}

	Bool hasTouchDevice() const;

	/// Populate the key and button with the new state
	Error handleEvents();

	// Add a new event
	// It's thread-safe
	void addEvent(InputEvent eventId)
	{
		m_events[eventId].fetchAdd(1);
	}

	// Get the times an event was triggered and resets the counter
	// It's thread-safe
	U32 getEvent(InputEvent eventId) const
	{
		return m_events[eventId].exchange(0);
	}

	/// Get some easy to digest input from the keyboard.
	CString getTextInput() const
	{
		return &m_textInput[0];
	}

protected:
	Array<I32, U(KeyCode::kCount)> m_keys;

	Array<I32, U(MouseButton::kCount)> m_mouseBtns;
	Vec2 m_mousePosNdc;
	Vec2 m_prevMousePosNdc;

	Array<I32, U(TouchPointer::kCount)> m_touchPointers;
	Array<Vec2, U(TouchPointer::kCount)> m_touchPointerPosNdc;

	mutable Array<Atomic<U32>, U(InputEvent::kCount)> m_events;

	/// The keybord input as ascii.
	static constexpr U32 kMaxTexInput = 256;
	Array<Char, kMaxTexInput> m_textInput;

	Bool m_lockCurs = false;

	Input()
	{
		reset();
	}

	void reset()
	{
		zeroMemory(m_keys);
		zeroMemory(m_mouseBtns);
		m_mousePosNdc = m_prevMousePosNdc = Vec2(-1.0f);
		zeroMemory(m_events);
		zeroMemory(m_textInput);
		zeroMemory(m_touchPointers);
		zeroMemory(m_touchPointerPosNdc);
	}
};

template<>
template<>
Input& MakeSingletonPtr<Input>::allocateSingleton<>();

template<>
void MakeSingletonPtr<Input>::freeSingleton();

} // end namespace anki
