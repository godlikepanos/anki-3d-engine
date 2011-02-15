#include <algorithm>
#include "Object.h"
#include "Assert.h"
#include "Exception.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Object::Object(Object* parent):
	objParent(NULL)
{
	if(parent != NULL)
	{
		parent->addChild(this);
	}
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
Object::~Object()
{
	if(objParent != NULL)
	{
		objParent->removeChild(this);
	}

	// delete all children
	for(Container::reverse_iterator it=objChilds.rbegin(); it!=objChilds.rend(); it++)
	{
		delete *it;
	}
}


//======================================================================================================================
// addChild                                                                                                            =
//======================================================================================================================
void Object::addChild(Object* child)
{
	ASSERT(child != NULL);
	ASSERT(child->objParent == NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


//======================================================================================================================
// removeChild                                                                                                         =
//======================================================================================================================
void Object::removeChild(Object* child)
{
	ASSERT(child != NULL);

	Container::iterator it = std::find(objChilds.begin(), objChilds.end(), child);

	if(it == objChilds.end())
	{
		throw EXCEPTION("Internal error");
	}

	objChilds.erase(it);
	child->objParent = NULL;
}
