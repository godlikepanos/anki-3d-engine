#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Vec.h"


/**
 * A class for automatic garbage collection. Cause we -the programmers- get bored when it comes to deallocation. Dont
 * even think to put as a parent an object that has not created dynamically
 */
class Object
{
	public:
		Object(Object* parent = NULL);

		/**
		 * Delete childs from the last entered to the first
		 */
		virtual ~Object();

		/**
		 * @name Accessors
		 */
		/**@{*/
		const Object* getObjParent() const {return objParent;}
		const Vec<Object*> getObjChilds() const {return objChilds;} ///< Get the childs Vec
		/**@}*/

	private:
		Object* objParent;
		Vec<Object*> objChilds;

		void addChild(Object* child);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Object::Object(Object* parent):
	objParent(NULL)
{
	if(parent != NULL)
		parent->addChild(this);
}


inline Object::~Object()
{
	for(Vec<Object*>::reverse_iterator it=objChilds.rbegin(); it!=objChilds.rend(); it++)
	{
		delete *it;
	}
}


inline void Object::addChild(Object* child)
{
	DEBUG_ERR(child == NULL);
	DEBUG_ERR(child->objParent != NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


#endif
