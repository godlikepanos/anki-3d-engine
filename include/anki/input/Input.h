// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_INPUT_INPUT_H
#define ANKI_INPUT_INPUT_H

#include "anki/Math.h"
#include "anki/util/Singleton.h"
#include "anki/util/Array.h"
#include "anki/util/StdTypes.h"
#include "anki/input/KeyCode.h"
#include <memory>

namespace anki {

struct InputImpl;
class NativeWindow;

/// Handle the input and other events
///
/// @note All positions are in NDC space
class Input
{
public:
	enum Event
	{
		WINDOW_FOCUS_LOST_EVENT,
		WINDOW_FOCUS_GAINED_EVENT,
		WINDOW_CLOSED_EVENT,
		EVENTS_COUNT
	};

	Input()
	{
		reset();
	}
	~Input();

	/// @name Acessors
	/// @{
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
	U getEvent(Event eventId) const
	{
		return m_events[eventId];
	}
	/// @}

	/// Initialize the platform's input system
	void init(NativeWindow* nativeWindow);

	/// Reset the keys and mouse buttons
	void reset();

	/// Populate the key and button with the new state
	void handleEvents();

	/// Move the mouse cursor to a position inside the window. Useful for 
	/// locking the cursor into a fixed location (eg in the center of the 
	/// screen)
	void moveCursor(const Vec2& posNdc);

	/// Hide the mouse cursor
	void hideCursor(Bool hide);

	/// Lock mouse to (0, 0)
	void lockCursor(Bool lock)
	{
		m_lockCurs = lock;
	}

	/// Add a new event
	void addEvent(Event eventId)
	{
		++m_events[eventId];
	}

private:
	NativeWindow* m_nativeWindow = nullptr;

	/// @name Keys and btns
	/// @{

	/// Shows the current key state
	/// - 0 times: unpressed
	/// - 1 times: pressed once
	/// - >1 times: Kept pressed 'n' times continuously
	Array<U32, U(KeyCode::COUNT)> m_keys;

	/// Mouse btns. Supporting 3 btns & wheel. @see keys
	Array<U32, 8> m_mouseBtns;
	/// @}

	Vec2 m_mousePosNdc = Vec2(2.0); ///< The coords are in the NDC space

	Array<U8, EVENTS_COUNT> m_events;

	std::shared_ptr<InputImpl> m_impl;

	Bool8 m_lockCurs = false;
};

typedef Singleton<Input> InputSingleton;

} // end namespace anki

#endif
