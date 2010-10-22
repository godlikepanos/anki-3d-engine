#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL_scancode.h>
#include "Vec.h"
#include "Math.h"
#include "Object.h"


/// Handle the SDL input
class Input: public Object
{
	public:
		Input(Object* parent);

		// keys and btns
		Vec<short> keys;  ///< Shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continuously
		Vec<short> mouseBtns; ///< Mouse btns. Supporting 3 btns & wheel. @see keys

		void reset();
		void handleEvents();

		// mouse stuff
		Vec2 mousePosNdc; ///< The coords are in the NDC space
		Vec2 mousePos;     ///< The coords are in the window space. (0, 0) is in the upper left corner
		Vec2 mouseVelocity;
		bool warpMouse;
		bool hideCursor;

	private:
		Object* parentApp; ///< Hold the parrent here cause we use him
};


#endif
