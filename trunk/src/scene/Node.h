#ifndef _NODE_H_
#define _NODE_H_

#include <memory>
#include "common.h"
#include "gmath.h"


class bvolume_t;
class Material;
class Controller;


/// Scene Node
class Node
{
	// data
	public:
		enum Type
		{
			NT_LIGHT,
			NT_CAMERA,
			NT_MESH,
			NT_SKELETON,
			NT_SKEL_MODEL
		};

		vec3_t translationLspace;
		mat3_t rotationLspace;
		float  scaleLspace;

		vec3_t translationWspace;
		mat3_t rotationWspace;
		float  scaleWspace;

		mat4_t transformationWspace;

		Node* parent;
		Vec<Node*> childs;

		Type type;

		bvolume_t* bvolumeLspace;
		bvolume_t* bvolumeWspace;

		bool isGroupNode;

	// funcs
	private:
		void commonConstructorCode(); ///< Cause we cannot call constructor from other constructor
		
	public:
		Node( Type type_ ): type(type_) { commonConstructorCode(); }
		Node( Type type_, Node* parent ): type(type_) { commonConstructorCode(); parent->addChild(this); }
		virtual ~Node();
		virtual void render() = 0;
		virtual void init( const char* ) = 0; ///< init using a script
		virtual void deinit() = 0;
		virtual void updateWorldStuff() { updateWorldTransform(); } ///< This update happens only when the object gets moved. Override it if you want more
		void updateWorldTransform();
		void rotateLocalX( float ang_degrees ) { rotationLspace.RotateXAxis( ang_degrees ); }
		void rotateLocalY( float ang_degrees ) { rotationLspace.RotateYAxis( ang_degrees ); }
		void rotateLocalZ( float ang_degrees ) { rotationLspace.RotateZAxis( ang_degrees ); }
		void moveLocalX( float distance );
		void moveLocalY( float distance );
		void moveLocalZ( float distance );
		void addChild( Node* node );
		void removeChild( Node* node );
		void setLocalTransformation( const vec3_t& t, const mat3_t& r, float s ) { translationLspace=t; rotationLspace=r; scaleLspace=s; }
};


#endif
