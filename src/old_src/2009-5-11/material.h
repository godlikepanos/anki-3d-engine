#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "shaders.h"
#include "math.h"
#include "texture.h"
#include "engine_class.h"


class material_t: public data_class_t
{
	public:
		static const uint TEXTURE_UNITS_NUM = 4;
		texture_t* textures[TEXTURE_UNITS_NUM];
		uint textures_num;

		shader_prog_t* shader;

		enum
		{
			AMBIENT_COL,
			DIFFUSE_COL,
			SPECULAR_COL,
			COLS_NUM
		};

		vec4_t colors[COLS_NUM];
		float shininess;

		static material_t* used_previously;

		 material_t(): shader(NULL), shininess(0.0)
		{
			for( uint i=0; i<TEXTURE_UNITS_NUM; i++ )
				textures[i] = NULL;
		}

		~material_t() {}

		bool Load( const char* filename );
		void Unload();

		void Use();
};


#endif
