#ifndef _NODE_H_
#define _NODE_H_

#include <memory>
#include "Common.h"
#include "Math.h"


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

		Vec3 translationLspace;
		Mat3 rotationLspace;
		float  scaleLspace;

		Vec3 translationWspace;
		Mat3 rotationWspace;
		float  scaleWspace;

		Mat4 transformationWspace;

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
		void rotateLocalX( float angDegrees ) { rotationLspace.rotateXAxis( angDegrees ); }
		void rotateLocalY( float angDegrees ) { rotationLspace.rotateYAxis( angDegrees ); }
		void rotateLocalZ( float angDegrees ) { rotationLspace.rotateZAxis( angDegrees ); }
		void moveLocalX( float distance );
		void moveLocalY( float distance );
		void moveLocalZ( float distance );
		void addChild( Node* node );
		void removeChild( Node* node );
		void setLocalTransformation( const Vec3& t, const Mat3& r, float s ) { translationLspace=t; rotationLspace=r; scaleLspace=s; }
};


#endif
