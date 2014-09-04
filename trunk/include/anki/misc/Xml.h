// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_XML_H
#define ANKI_XML_H

#include "anki/util/Exception.h"
#include "anki/util/String.h"
#include "anki/Math.h"
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
#endif

namespace anki {

/// XML element
class XmlElement
{
	friend class XmlDocument;

public:
	XmlElement()
	:	m_el(nullptr)
	{}

	XmlElement(const XmlElement& b)
	:	m_el(b.m_el),
		m_alloc(b.m_alloc)
	{}

	/// If element has something return true
	operator Bool() const
	{
		return m_el != nullptr;
	}

	/// Copy
	XmlElement& operator=(const XmlElement& b)
	{
		m_el = b.m_el;
		return *this;
	}

	/// Return the text inside a tag
	CString getText() const
	{
		check();
		return CString(m_el->GetText());
	}

	/// Return the text inside as an int
	I64 getInt() const;

	/// Return the text inside as a float
	F64 getFloat() const;

	/// Get a number of floats
	Vector<F64, StackAllocator<F64>> getFloats() const;

	/// Return the text inside as a Mat4
	Mat4 getMat4() const;

	/// Return the text inside as a Vec3
	Vec3 getVec3() const;

	/// Return the text inside as a Vec4
	Vec4 getVec4() const;

	/// Get a child element quietly
	XmlElement getChildElementOptional(const CString& name) const;

	/// Get a child element and throw exception if not found
	XmlElement getChildElement(const CString& name) const;

	/// Get the next element with the same name. Returns empty XmlElement if
	/// it reached the end of the list
	XmlElement getNextSiblingElement(const CString& name) const;

private:
	tinyxml2::XMLElement* m_el;
	StackAllocator<U8> m_alloc;

	void check() const
	{
		if(m_el == nullptr)
		{
			throw ANKI_EXCEPTION("Empty element");
		}
	}
};

/// XML document
class XmlDocument
{
public:
	void loadFile(const CString& filename, StackAllocator<U8>& alloc);

	XmlElement getChildElement(const CString& name)
	{
		XmlElement el;
		el.m_el = m_doc.FirstChildElement(&name[0]);
		el.m_alloc = m_alloc;
		return el;
	}

private:
	tinyxml2::XMLDocument m_doc;
	StackAllocator<U8> m_alloc;
};

} // end namespace anki

#endif
