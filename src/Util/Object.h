#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Vec.h"


/// A class for automatic garbage collection. Cause we -the programmers- get bored when it comes to deallocation. Dont
/// even think to put as a parent an object that has not created dynamically
class Object
{
	public:
		Object(Object* parent = NULL);

		/// Delete childs from the last entered to the first and update parent
		virtual ~Object();

	private:
		Object* objParent;
		Vec<Object*> objChilds;

		void addChild(Object* child);
		void removeChild(Object* child);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Object::Object(Object* parent):
	objParent(NULL)
{
	if(parent != NULL)
	{
		parent->addChild(this);
	}
}


inline Object::~Object()
{
	if(objParent != NULL)
	{
		objParent->removeChild(this);
	}

	// delete all children
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


inline void Object::removeChild(Object* child)
{
	DEBUG_ERR(child == NULL);

	Vec<Object*>::iterator it = find(objChilds.begin(), objChilds.end(), child);

	if(it == objChilds.end())
	{
		ERROR("Internal error");
		return;
	}

	objChilds.erase(it);
	child->objParent = NULL;
}


#endif
