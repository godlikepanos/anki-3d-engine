#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "common.h"
#include "gmath.h"
#include "engine_class.h"

class bvolume_t;
class material_t;

class object_t: public data_user_class_t
{
	public:
		vec3_t translation_lspace;
		float  scale_lspace;
		mat3_t rotation_lspace;

		vec3_t translation_wspace;
		float  scale_wspace;
		mat3_t rotation_wspace;

		mat4_t transformation_wspace;

		object_t* parent;
		vec_t<object_t*> childs;

		enum type_e
		{
			LIGHT,
			CAMERA,
			MODEL,
			MESH,
			CUSTOM
		};
		type_e type;

		bvolume_t* bvolume_lspace;
		bvolume_t* bvolume_wspace;

		// funcs
		 object_t( type_e type_ );
		~object_t() {};
		virtual void Render() = 0; ///< Later make that const
		virtual void RenderDepth() = 0; ///< Later make that const
		virtual void UpdateWorldStuff() { UpdateWorldTransform(); }; ///< This update happens only when the object gets moved. Override it if you want more
		const char* GetTypeStr() const;
		void RenderAxis();
		void UpdateWorldTransform();
		void RotateLocalX( float ang_degrees ) { rotation_lspace.RotateXAxis( ang_degrees ); }
		void RotateLocalY( float ang_degrees ) { rotation_lspace.RotateYAxis( ang_degrees ); }
		void RotateLocalZ( float ang_degrees ) { rotation_lspace.RotateZAxis( ang_degrees ); }
		void MoveLocalX( float distance );
		void MoveLocalY( float distance );
		void MoveLocalZ( float distance );
		void AddChilds( uint amount, ... );
		void MakeParent( object_t* obj );
};


#endif

