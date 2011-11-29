#include <algorithm>
#include <boost/foreach.hpp>

#include "anki/core/Object.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Object::Object(Object* parent)
	: objParent(NULL)
{
	if(parent != NULL)
	{
		parent->addChild(this);
	}
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Object::~Object()
{
	if(objParent != NULL)
	{
		objParent->removeChild(this);
	}

	// delete all children
	BOOST_REVERSE_FOREACH(Object* child, objChilds)
	{
		delete child;
	}
}


//==============================================================================
// addChild                                                                    =
//==============================================================================
void Object::addChild(Object* child)
{
	ANKI_ASSERT(child != NULL);
	ANKI_ASSERT(child->objParent == NULL); // Child already has parent

	child->objParent = this;
	objChilds.push_back(child);
}


//==============================================================================
// removeChild                                                                 =
//==============================================================================
void Object::removeChild(Object* child)
{
	ANKI_ASSERT(child != NULL);

	Container::iterator it = std::find(objChilds.begin(), objChilds.end(),
		child);

	if(it == objChilds.end())
	{
		throw ANKI_EXCEPTION("Internal error");
	}

	objChilds.erase(it);
	child->objParent = NULL;
}


} // end namespace
