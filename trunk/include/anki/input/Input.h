#ifndef ANKI_INPUT_INPUT_H
#define ANKI_INPUT_INPUT_H

#include "anki/math/Math.h"
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
	Input()
	{
		reset();
	}
	~Input();

	/// @name Acessors
	/// @{
	U32 getKey(U32 i) const
	{
		return keys[i];
	}

	U32 getMouseButton(U32 i) const
	{
		return mouseBtns[i];
	}

	const Vec2& getMousePosition() const
	{
		return mousePosNdc;
	}
	/// @}

	void init(NativeWindow* nativeWindow);
	void reset();
	void handleEvents();
	void moveMouse(const Vec2& posNdc);
	void hideCursor(Bool hide);

private:
	NativeWindow* nativeWindow = nullptr;

	/// @name Keys and btns
	/// @{

	/// Shows the current key state
	/// - 0 times: unpressed
	/// - 1 times: pressed once
	/// - >1 times: Kept pressed 'n' times continuously
	Array<U32, KC_COUNT> keys;

	/// Mouse btns. Supporting 3 btns & wheel. @see keys
	Array<U32, 8> mouseBtns;
	/// @}

	Vec2 mousePosNdc; ///< The coords are in the NDC space

	Array<U16, 256> nativeKeyToAnki;

	std::shared_ptr<InputImpl> impl;
};

typedef Singleton<Input> InputSingleton;

} // end namespace anki

#endif
