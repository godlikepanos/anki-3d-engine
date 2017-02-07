// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/misc/Common.h>
#include <anki/util/String.h>
#include <anki/util/DynamicArray.h>
#include <anki/Math.h>
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#error "Wrong tinyxml2 included"
#endif

namespace anki
{

/// @addtogroup misc
/// @{

/// XML element.
class XmlElement
{
	friend class XmlDocument;

public:
	XmlElement()
		: m_el(nullptr)
	{
	}

	XmlElement(const XmlElement& b)
		: m_el(b.m_el)
		, m_alloc(b.m_alloc)
	{
	}

	XmlElement(const tinyxml2::XMLElement* el, GenericMemoryPoolAllocator<U8> alloc)
		: m_el(el)
		, m_alloc(alloc)
	{
	}

	/// If element has something return true
	operator Bool() const
	{
		return m_el != nullptr;
	}

	/// Copy
	XmlElement& operator=(const XmlElement& b)
	{
		m_el = b.m_el;
		m_alloc = b.m_alloc;
		return *this;
	}

	/// Return the text inside a tag
	ANKI_USE_RESULT Error getText(CString& out) const;

	/// Return the text inside as an int
	ANKI_USE_RESULT Error getI64(I64& out) const;

	/// Return the text inside as a float
	ANKI_USE_RESULT Error getF64(F64& out) const;

	/// Return the text inside as a float
	ANKI_USE_RESULT Error getF32(F32& out) const
	{
		F64 outd;
		ANKI_CHECK(getF64(outd));
		out = outd;
		return ErrorCode::NONE;
	}

	/// Get a number of floats
	ANKI_USE_RESULT Error getFloats(DynamicArrayAuto<F64>& out) const;

	/// Return the text inside as a Mat4
	ANKI_USE_RESULT Error getMat4(Mat4& out) const;

	/// Return the text inside as a Vec3
	ANKI_USE_RESULT Error getVec3(Vec3& out) const;

	/// Return the text inside as a Vec4
	ANKI_USE_RESULT Error getVec4(Vec4& out) const;

	/// Get a child element quietly
	ANKI_USE_RESULT Error getChildElementOptional(const CString& name, XmlElement& out) const;

	/// Get a child element and print error if not found
	ANKI_USE_RESULT Error getChildElement(const CString& name, XmlElement& out) const;

	/// Get the next element with the same name. Returns empty XmlElement if it reached the end of the list.
	ANKI_USE_RESULT Error getNextSiblingElement(const CString& name, XmlElement& out) const;

	/// Get the number of sibling elements of this node.
	/// @note The sibling elements share the same name.
	ANKI_USE_RESULT Error getSiblingElementsCount(U32& out) const;

private:
	const tinyxml2::XMLElement* m_el;
	GenericMemoryPoolAllocator<U8> m_alloc;

	ANKI_USE_RESULT Error check() const;
};

/// XML document
class XmlDocument
{
public:
	static CString XML_HEADER;

	ANKI_USE_RESULT Error loadFile(const CString& filename, GenericMemoryPoolAllocator<U8> alloc);

	ANKI_USE_RESULT Error parse(const CString& xmlText, GenericMemoryPoolAllocator<U8> alloc);

	ANKI_USE_RESULT Error getChildElement(const CString& name, XmlElement& out) const;

private:
	tinyxml2::XMLDocument m_doc;
	GenericMemoryPoolAllocator<U8> m_alloc;
};
/// @}

} // end namespace anki
