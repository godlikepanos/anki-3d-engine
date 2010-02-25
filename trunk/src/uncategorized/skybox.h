#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "common.h"
#include "Texture.h"
#include "Math.h"

class ShaderProg;

class skybox_t
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

		Texture* textures[6];
		Texture* noise;
		ShaderProg* shader;

		float rotation_ang;

	public:
		skybox_t() { rotation_ang=0.0; }

		bool load( const char* filenames[6] );
		void Render( const Mat3& rotation );
};


#endif
