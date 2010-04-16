#ifndef _APP_H_
#define _APP_H_

#include "Common.h"


/**
 * @brief This class holds all the global objects of the application. Its also responsible for some of the SDL stuff.
 * It should be singleton
 */
class App
{
	PROPERTY_R( uint, desktopW, getDesktopWidth ) ///< @ref PROPERTY_R : The desktop width at App initialization
	PROPERTY_R( uint, desktopH, getDesktopHeight ) ///< @ref PROPERTY_R : The desktop height at App initialization
	PROPERTY_R( uint, windowW, getWindowWidth ) ///< @ref PROPERTY_R : The main window width
	PROPERTY_R( uint, windowH, getWindowHeight ) ///< @ref PROPERTY_R : The main window height

	PROPERTY_RW( class Scene*, scene, setScene, getScene ) ///< @ref PROPERTY_RW : Pointer to the current scene
	PROPERTY_RW( class Camera*, activeCam, setActiveCam, getActiveCam ) ///< @ref PROPERTY_RW : Pointer to the current camera

	private:
		static bool isCreated; ///< A flag to ensure one @ref App instance
		uint time;
		class SDL_Surface* mainSurf; ///< SDL stuff
		class SDL_Surface* iconImage; ///< SDL stuff

	public:
		uint timerTick;

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
		static uint getTicks();
};


#endif
