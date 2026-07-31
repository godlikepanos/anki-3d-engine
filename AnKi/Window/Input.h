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

namespace anki {

// Keyboard scancodes taken from SDL
enum class KeyCode
{
	kUnknown = 0,

#define ANKI_KEY_CODE(ak, sdl) k##ak,
#include <AnKi/Window/KeyCode.def.h>
#undef ANKI_KEY_CODE

	kCount,
	kFirst = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(KeyCode)

enum class MouseButton : U8
{
	kLeft,
	kMiddle,
	kRight,
	kScrollUp,
	kScrollDown,

	kCount
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MouseButton)

enum class TouchPointer : U8
{
	k0,
	k1,
	k2,
	k3,
	k4,
	k5,
	k6,
	k7,
	k8,
	k9,
	k10,
	k11,
	k12,
	k13,
	k14,
	k15,

	kCount,
	kFirst = k0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TouchPointer)

// Gamepad buttons taken from SDL
enum class GamepadButton : U8
{
	kSouth, // Bottom face button. Xbox A, PlayStation cross, Switch B
	kEast, // Right face button. Xbox B, PlayStation circle, Switch A
	kWest, // Left face button. Xbox X, PlayStation square, Switch Y
	kNorth, // Top face button. Xbox Y, PlayStation triangle, Switch X
	kBack,
	kGuide, // Vendor/home button. The OS or Steam commonly swallows it before the app sees it, so don't make it required
	kStart,
	kLeftStick, // Clicking the left stick in
	kRightStick,
	kLeftShoulder, // The digital bumpers. The analog triggers are axes, see GamepadTrigger
	kRightShoulder,
	kDpadUp, // The dpad is 4 discrete buttons and not an axis
	kDpadDown,
	kDpadLeft,
	kDpadRight,
	kMisc1, // Share/capture/microphone, depending on the vendor
	kRightPaddle1, // Back paddles. Only on Xbox Elite and DualSense Edge class pads
	kLeftPaddle1,
	kRightPaddle2,
	kLeftPaddle2,
	kTouchpad, // Clicking the PS4/PS5 touchpad
	kMisc2, // The rest have no portable meaning
	kMisc3,
	kMisc4,
	kMisc5,
	kMisc6,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GamepadButton)

enum class GamepadStick : U8
{
	kLeft,
	kRight,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GamepadStick)

enum class GamepadTrigger : U8
{
	kLeft,
	kRight,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GamepadTrigger)

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

// Handle the input and other events
// Note: All positions are in NDC space
class Input : public MakeSingletonPtr<Input>
{
	template<typename>
	friend class MakeSingletonPtr;

public:
	Error init();

	// Shows the current key state
	// 0: Key resting
	// 1: Ley pressed once
	// >1: Kept pressed 'n' times continuously
	// <0: Key up
	I32 getKey(KeyCode i) const
	{
		return m_keys[i];
	}

	// See getKey()
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

	// Move the mouse cursor to a position inside the window. Useful for locking the cursor into a fixed location (eg in the center of the screen)
	// It's thread-safe
	void moveMouseNdc(Vec2 posNdc)
	{
		LockGuard lock(m_requests.m_lock);
		m_requests.m_mousePosNdc = posNdc.clamp(-1.0f, 1.0f);
	}

	// Lock the mouse to window center. Useful for FPS/TPS games
	// It's thread safe
	void lockMouseWindowCenter(Bool lock)
	{
		m_lockMouse.store(lock);
	}

	// Hide the mouse cursor
	// It's thread-safe
	void hideMouseCursor(Bool hide)
	{
		m_requests.m_hideCursor.store(I8(hide));
	}

	// Change the shape of the cursor
	// It's thread-safe
	void setMouseCursor(MouseCursor cursor)
	{
		ANKI_ASSERT(cursor < MouseCursor::kCount);
		LockGuard lock(m_requests.m_lock);
		m_requests.m_mouseCursor = cursor;
	}

	// See getKey()
	I32 getTouchPointer(TouchPointer p) const
	{
		return m_touchPointers[p];
	}

	Vec2 getTouchPointerNdcPosition(TouchPointer p) const
	{
		return m_touchPointerPosNdc[p];
	}

	Bool hasTouchDevice() const;

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

	// Get some easy to digest input from the keyboard.
	CString getTextInput() const
	{
		return &m_textInput[0];
	}

	Bool hasGamepad() const;

	// See getKey()
	I32 getGamepadButton(GamepadButton b) const
	{
		return m_gamepadBtns[b];
	}

	// Deadzoned and rescaled so that the magnitude ramps from 0 at the deadzone edge to 1 at full deflection. +Y is up, unlike the raw backend value
	Vec2 getGamepadStick(GamepadStick s) const
	{
		return m_gamepadSticks[s];
	}

	// In [0, 1]. The triggers are analog, unlike the shoulder buttons
	F32 getGamepadTrigger(GamepadTrigger t) const
	{
		return m_gamepadTriggers[t];
	}

	// Populate the key and button with the new state
	ANKI_INTERNAL Error handleEvents();

protected:
	Array<I32, U(KeyCode::kCount)> m_keys;

	Array<I32, U(MouseButton::kCount)> m_mouseBtns;
	Vec2 m_mousePosNdc = Vec2(-1.0f);
	Vec2 m_prevMousePosNdc = Vec2(-1.0f);

	Array<I32, U(TouchPointer::kCount)> m_touchPointers;
	Array<Vec2, U(TouchPointer::kCount)> m_touchPointerPosNdc;

	Array<I32, U(GamepadButton::kCount)> m_gamepadBtns;
	Array<Vec2, U(GamepadStick::kCount)> m_gamepadSticks;
	Array<F32, U(GamepadTrigger::kCount)> m_gamepadTriggers;

	mutable Array<Atomic<U32>, U(InputEvent::kCount)> m_events;

	// The keybord input as ascii.
	static constexpr U32 kMaxTexInput = 256;
	Array<Char, kMaxTexInput> m_textInput;

	MouseCursor m_mouseCursor = MouseCursor::kArrow;

	Atomic<Bool> m_lockMouse = {false};

	// Requests are deferred until handleEvents() because most backends are not multi-threaded
	class Requests
	{
	public:
		Atomic<I8> m_hideCursor = {-1};
		MouseCursor m_mouseCursor = MouseCursor::kCount;
		Vec2 m_mousePosNdc = Vec2(kMaxF32);
		SpinLock m_lock;
	} m_requests;

	Input()
	{
		zeroMemory(m_keys);
		zeroMemory(m_mouseBtns);
		zeroMemory(m_touchPointers);
		zeroMemory(m_touchPointerPosNdc);
		zeroMemory(m_gamepadBtns);
		zeroMemory(m_gamepadSticks);
		zeroMemory(m_gamepadTriggers);
		zeroMemory(m_events);
		zeroMemory(m_textInput);
	}
};

template<>
template<>
Input& MakeSingletonPtr<Input>::allocateSingleton<>();

template<>
void MakeSingletonPtr<Input>::freeSingleton();

} // end namespace anki
