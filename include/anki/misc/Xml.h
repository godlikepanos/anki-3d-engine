#ifndef ANKI_XML_H
#define ANKI_XML_H

#include "anki/util/Exception.h"
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
#endif

namespace anki {

/// XXX
struct XmlElement
{
	friend class XmlDocument;
public:
	XmlElement()
	{}
	XmlElement(const XmlElement& b)
		: el(b.el)
	{}

	const char* getText() const
	{
		return el->GetText();
	}

	XmlElement getChildElementOptional(const char* name)
	{
		XmlElement out;
		out.el = el->FirstChildElement(name);
		return out;
	}

	XmlElement getChildElement(const char* name)
	{
		XmlElement out = getChildElementOptional(name);
		if(!out)
		{
			throw ANKI_EXCEPTION("Cannot find <" + name + ">");
		}
		return out;
	}

	XmlElement getNextSiblingElement(const char* name)
	{
		XmlElement out;
		out.el = el->NextSiblingElement(name);
		return out;
	}

	/// If element has something return true
	operator bool() const
	{
		return el != nullptr;
	}

	XmlElement& operator=(const XmlElement& b)
	{
		el = b.el;
		return *this;
	}

private:
	tinyxml2::XMLElement* el = nullptr;
};

/// XXX
class XmlDocument
{
public:
	void loadFile(const char* filename)
	{
		if(doc.LoadFile(filename))
		{
			throw ANKI_EXCEPTION("Cannot parse file");
		}
	}

	XmlElement getChildElement(const char* name)
	{
		XmlElement el;
		el.el = doc.FirstChildElement(name);
		return el;
	}

private:
	tinyxml2::XMLDocument doc;
};

} // end namespace anki

#endif
