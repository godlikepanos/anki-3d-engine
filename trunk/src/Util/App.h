#ifndef _APP_H_
#define _APP_H_

#include <SDL/SDL.h>
#include "Common.h"


class Scene;
class Camera;


class App
{
	PROPERTY_R( uint, desktopW, getDesktopW )
	PROPERTY_R( uint, desktopH, getDesktopH )

	private:
		uint time;
		SDL_Surface* mainSurf;
		SDL_Surface* iconImage;

	public:
		uint timerTick;
		uint windowW;
		uint windowH;

		Scene*  scene;
		Camera* activeCam;

		App();
		void initWindow();
		void quitApp( int code );
		void waitForNextFrame();
		void togleFullScreen();
		static void printAppInfo();

		/// Gets the number of milliseconds since SDL library initialization
		static uint getTicks() { return SDL_GetTicks(); }
};


#endif
