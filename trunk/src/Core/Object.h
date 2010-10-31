#ifndef OBJECT_H
#define OBJECT_H

#include "Vec.h"


/// A class for automatic garbage collection. Cause we -the programmers- get bored when it comes to deallocation. Dont
/// even think to put as a parent an object that has not created dynamically
class Object
{
	public:
		/// Calls addChild if parent is not NULL
		/// @exception Exception
		Object(Object* parent);

		/// Delete childs from the last entered to the first and update parent
		virtual ~Object();

	private:
		Object* objParent;
		Vec<Object*> objChilds;

		void addChild(Object* child);
		void removeChild(Object* child);
};


#endif
