#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "common.h"
#include "texture.h"
#include "math.h"

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

		texture_t* textures[6];
		texture_t* noise;
		shader_prog_t* shader;

		float rotation_ang;

	public:
		skybox_t() { rotation_ang=0.0; }

		bool Load( const char* filenames[6] );
		void Render( const mat3_t& rotation );
};


#endif
