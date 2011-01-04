#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL_scancode.h>
#include <boost/array.hpp>
#include "Math.h"


/// Handle the SDL input
class Input
{
	public:
		/// Singleton stuff
		static Input& getInstance();

		// keys and btns
		boost::array<short, SDL_NUM_SCANCODES> keys;  ///< Shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continuously
		boost::array<short, 8> mouseBtns; ///< Mouse btns. Supporting 3 btns & wheel. @see keys

		void reset();
		void handleEvents();

		// mouse stuff
		Vec2 mousePosNdc; ///< The coords are in the NDC space
		Vec2 mousePos;     ///< The coords are in the window space. (0, 0) is in the upper left corner
		Vec2 mouseVelocity;
		bool warpMouse;
		bool hideCursor;

	private:
		static Input* instance;

		/// @name Ensure its singleton
		/// @{
		Input() {init();}
		Input(const Input&) {init();}
		void operator=(const Input&) {}
		/// @}

		void init();
};


inline Input& Input::getInstance()
{
	if(instance == NULL)
	{
		instance = new Input();
	}
	return *instance;
}


#endif
