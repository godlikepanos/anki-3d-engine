#ifndef ANKI_XML_H
#define ANKI_XML_H

#include "anki/util/Exception.h"
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
#endif
#include <string>

namespace anki {

/// XML element
struct XmlElement
{
	friend class XmlDocument;
public:
	XmlElement()
	{}
	XmlElement(const XmlElement& b)
		: el(b.el)
	{}

	/// If element has something return true
	operator bool() const
	{
		return el != nullptr;
	}

	/// Copy
	XmlElement& operator=(const XmlElement& b)
	{
		el = b.el;
		return *this;
	}

	/// Return the text inside a tag
	const char* getText() const
	{
		check();
		return el->GetText();
	}

	/// Return the text inside as a string
	int getInt() const;

	/// Get a child element quietly
	XmlElement getChildElementOptional(const char* name) const;

	/// Get a child element and throw exception if not found
	XmlElement getChildElement(const char* name) const;

	/// Get the next element with the same name. Returns empty XmlElement if
	/// it reached the end of the list
	XmlElement getNextSiblingElement(const char* name) const;

private:
	tinyxml2::XMLElement* el = nullptr;

	void check() const
	{
		if(el == nullptr)
		{
			throw ANKI_EXCEPTION("Empty element");
		}
	}
};

/// XML document
class XmlDocument
{
public:
	void loadFile(const char* filename)
	{
		if(doc.LoadFile(filename))
		{
			throw ANKI_EXCEPTION("Cannot parse file. Reason: "
				+ ((doc.GetErrorStr1() == nullptr)
				? "unknown" : doc.GetErrorStr1()));
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
