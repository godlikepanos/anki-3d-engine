#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Vec.h"


/**
 * A class for automatic garbage collection. Cause we -the programmers- get bored when it comes to deallocation
 */
class Object
{
	public:
		Object(Object* parent = NULL);
		virtual ~Object();

		/**
		 * @name Accessors
		 */
		/**@{*/
		const Object* getObjParent() const;
		const Vec<Object*> getObjChilds() const; ///< Get the childs Vec
		/**@}*/

	private:
		Object* objParent;
		Vec<Object*> objChilds;

		void addChild(Object* child);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Object::Object(Object* parent)
{
	if(parent)
		parent->addChild(this);
}


inline Object::~Object()
{
	for(Vec<Object*>::iterator it=objChilds.begin(); it!=objChilds.end(); it++)
	{
		delete *it;
	}
}


inline const Object* Object::getObjParent() const
{
	return objParent;
}


inline const Vec<Object*> Object::getObjChilds() const
{
	return objChilds;
}


inline void Object::addChild(Object* child)
{
	DEBUG_ERR(child == NULL);
	DEBUG_ERR(child->objParent != NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


#endif
