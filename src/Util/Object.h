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
		const Object* getParent() const;
		const Vec<Object*> getChilds() const; ///< Get the childs Vec
		/**@}*/

	private:
		Object* parent;
		Vec<Object*> childs;

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
	for(Vec<Object*>::iterator it=childs.begin(); it!=childs.end(); it++)
	{
		delete *it;
	}
}


inline const Object* Object::getParent() const
{
	return parent;
}


inline const Vec<Object*> Object::getChilds() const
{
	return childs;
}


inline void Object::addChild(Object* child)
{
	DEBUG_ERR(child == NULL);
	DEBUG_ERR(child->parent != NULL); // Child already has parent

	child->parent = this;
	childs.push_back(child);
}


#endif
