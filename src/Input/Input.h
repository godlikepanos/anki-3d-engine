#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL_scancode.h>
#include <boost/array.hpp>
#include "Math/Math.h"
#include "Util/Singleton.h"
#include "Util/Accessors.h"


/// Handle the SDL input
class Input
{
	public:
		Input() {init();}

		/// @name Acessors
		/// @{
		short getKey(uint i) const {return keys[i];}
		short getMouseBtn(uint i) const {return mouseBtns[i];}
		GETTER_SETTER_BY_VAL(bool, warpMouseFlag, warpMouse, setWarpMouse)
		/// @}

		void reset();
		void handleEvents();

		// mouse stuff
		Vec2 mousePosNdc; ///< The coords are in the NDC space
		Vec2 mousePos;     ///< The coords are in the window space. (0, 0) is in the upper left corner
		Vec2 mouseVelocity;
		bool hideCursor;

	private:
		/// @name Keys and btns
		/// @{

		/// Shows the current key state
		/// - 0 times: unpressed
		/// - 1 times: pressed once
		/// - >1 times: Kept pressed 'n' times continuously
		boost::array<short, SDL_NUM_SCANCODES> keys;

		boost::array<short, 8> mouseBtns; ///< Mouse btns. Supporting 3 btns & wheel. @see keys
		/// @}

		bool warpMouseFlag;

		void init();
};


#endif
