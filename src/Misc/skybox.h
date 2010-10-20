#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Texture.h"
#include "Math.h"
#include "RsrcPtr.h"
#include "ShaderProg.h"

class Skybox
{
	protected:
		enum textures_e
		{
			FRONT,
			BACK,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

		RsrcPtr<Texture> textures[6];
		RsrcPtr<Texture> noise;
		RsrcPtr<ShaderProg> shader;

		float rotation_ang;

	public:
		Skybox() { rotation_ang=0.0; }

		bool load(const char* filenames[6]);
		void Render(const Mat3& rotation);
};


#endif
