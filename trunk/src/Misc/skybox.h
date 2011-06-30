#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Resources/Texture.h"
#include "Math/Math.h"
#include "Resources/RsrcPtr.h"
#include "Resources/ShaderProg.h"

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
