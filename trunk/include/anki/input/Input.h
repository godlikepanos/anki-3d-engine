#ifndef ANKI_INPUT_INPUT_H
#define ANKI_INPUT_INPUT_H

#include "anki/math/Math.h"
#include "anki/util/Singleton.h"
#include "anki/util/Array.h"

namespace anki {

class NativeWindow;

/// Handle the input and other events
class Input
{
public:
	Input()
	{}
	~Input()
	{}

	/// @name Acessors
	/// @{
	U32 getKey(U32 i) const
	{
		return keys[i];
	}

	U32 getMouseBtn(U32 i) const
	{
		return mouseBtns[i];
	}
	/// @}

	void init(NativeWindow* nativeWindow);
	void reset();
	void handleEvents();

private:
	NativeWindow* nativeWindow = nullptr;

	/// @name Keys and btns
	/// @{

	/// Shows the current key state
	/// - 0 times: unpressed
	/// - 1 times: pressed once
	/// - >1 times: Kept pressed 'n' times continuously
	Array<U32, 128> keys;

	/// Mouse btns. Supporting 3 btns & wheel. @see keys
	Array<U32, 8> mouseBtns;
	/// @}

	Vec2 mousePosNdc; ///< The coords are in the NDC space
};

typedef Singleton<Input> InputSingleton;

} // end namespace anki

#endif
