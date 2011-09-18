#ifndef INPUT_H
#define INPUT_H

#include "m/Math.h"
#include <SDL/SDL_scancode.h>
#include <boost/array.hpp>


/// Handle the SDL input
class Input
{
	public:
		Input() {init();}

		/// @name Acessors
		/// @{
		short getKey(uint i) const
		{
			return keys[i];
		}

		short getMouseBtn(uint i) const
		{
			return mouseBtns[i];
		}

		bool getWarpMouse() const
		{
			return warpMouseFlag;
		}
		bool& getWarpMouse()
		{
			return warpMouseFlag;
		}
		void setWarpMouse(const bool x)
		{
			warpMouseFlag = x;
		}
		/// @}

		void reset();
		void handleEvents();

		// mouse stuff
		Vec2 mousePosNdc; ///< The coords are in the NDC space
		/// The coords are in the window space. (0, 0) is in the upper left
		/// corner
		Vec2 mousePos;
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

		/// Mouse btns. Supporting 3 btns & wheel. @see keys
		boost::array<short, 8> mouseBtns;
		/// @}

		bool warpMouseFlag;

		void init();
};


#endif
