#ifndef _APP_H_
#define _APP_H_

#include <SDL/SDL.h>
#include "Common.h"


/**
 * @brief This class contains all the global objects of the application. Its also responsible for some of the SDL stuff.
 * It should be singleton
 */
class App
{
	PROPERTY_R( uint, desktopW, getDesktopW ) ///< Property: The desktop width at App initialization
	PROPERTY_R( uint, desktopH, getDesktopH ) ///< Property: The desktop height at App initialization
	PROPERTY_RW( class Scene*, scene, setScene, getScene ) ///< Property: Pointer to the current scene
	PROPERTY_RW( class Camera*, activeCam, setActiveCam, getActiveCam ) ///< Property: Pointer to the current camera

	private:
		static bool isCreated; ///< A flag to ensure one @ref App instance
		uint time;
		SDL_Surface* mainSurf;
		SDL_Surface* iconImage;

	public:
		uint timerTick;
		uint windowW;
		uint windowH;

		App();
		~App() {}
		void initWindow();
		void quitApp( int code );
		void waitForNextFrame();
		void togleFullScreen();
		static void printAppInfo();

		/**
		 * @return Returns the number of milliseconds since SDL library initialization
		 */
		static uint getTicks() { return SDL_GetTicks(); }
};


#endif
