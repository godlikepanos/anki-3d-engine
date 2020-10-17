// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/DynamicArray.h>
#include <anki/util/String.h>
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
#endif

namespace anki
{

/// @addtogroup util_file
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

	/// If element has something return true
	explicit operator Bool() const
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

	/// Return the text inside a tag. May return empty string.
	ANKI_USE_RESULT Error getText(CString& out) const;

	/// Return the text inside as a number.
	template<typename T>
	ANKI_USE_RESULT Error getNumber(T& out) const;

	/// Get a number of numbers.
	template<typename T>
	ANKI_USE_RESULT Error getNumbers(DynamicArrayAuto<T>& out) const;

	/// Get a fixed number of numbers.
	/// @tparam TArray A type that should have operator[] and getSize() methods implemented.
	template<typename TArray>
	ANKI_USE_RESULT Error getNumbers(TArray& out) const;

	/// Get a child element quietly
	ANKI_USE_RESULT Error getChildElementOptional(CString name, XmlElement& out) const;

	/// Get a child element and print error if not found
	ANKI_USE_RESULT Error getChildElement(CString name, XmlElement& out) const;

	/// Get the next element with the same name. Returns empty XmlElement if it reached the end of the list.
	ANKI_USE_RESULT Error getNextSiblingElement(CString name, XmlElement& out) const;

	/// Get the number of sibling elements of this node.
	/// @note The sibling elements share the same name.
	ANKI_USE_RESULT Error getSiblingElementsCount(U32& out) const;

	/// @name Get attributes optional
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	ANKI_USE_RESULT Error getAttributeTextOptional(CString name, CString& out, Bool& attribPresent) const;

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumbersOptional(CString name, DynamicArrayAuto<T>& out,
													  Bool& attribPresent) const;

	/// Get the attribute's value as a series of numbers.
	/// @tparam TArray A type that should have operator[] and getSize() methods implemented.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename TArray>
	ANKI_USE_RESULT Error getAttributeNumbersOptional(CString name, TArray& out, Bool& attribPresent) const;

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumberOptional(CString name, T& out, Bool& attribPresent) const;
	/// @}

	/// @name Get attributes
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	ANKI_USE_RESULT Error getAttributeText(CString name, CString& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeTextOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumbers(CString name, DynamicArrayAuto<T>& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumbersOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a series of numbers.
	/// @tparam TArray A type that should have operator[] and getSize() methods implemented.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename TArray>
	ANKI_USE_RESULT Error getAttributeNumbers(CString name, TArray& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumbersOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumber(CString name, T& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumberOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}
	/// @}

private:
	const tinyxml2::XMLElement* m_el;
	GenericMemoryPoolAllocator<U8> m_alloc;

	XmlElement(const tinyxml2::XMLElement* el, GenericMemoryPoolAllocator<U8> alloc)
		: m_el(el)
		, m_alloc(alloc)
	{
	}

	ANKI_USE_RESULT Error check() const;

	template<typename T>
	ANKI_USE_RESULT Error parseNumbers(CString txt, DynamicArrayAuto<T>& out) const;

	template<typename TArray>
	ANKI_USE_RESULT Error parseNumbers(CString txt, TArray& out) const;

	ANKI_USE_RESULT Error throwAttribNotFoundError(CString attrib, Bool found) const
	{
		if(!found)
		{
			ANKI_UTIL_LOGE("Attribute not found \"%s\"", &attrib[0]);
			return Error::USER_DATA;
		}
		else
		{
			return Error::NONE;
		}
	}
};

/// XML document.
class XmlDocument
{
public:
	static CString XML_HEADER;

	/// Parse from a file.
	ANKI_USE_RESULT Error loadFile(CString filename, GenericMemoryPoolAllocator<U8> alloc);

	/// Parse from a CString.
	ANKI_USE_RESULT Error parse(CString xmlText, GenericMemoryPoolAllocator<U8> alloc);

	ANKI_USE_RESULT Error getChildElement(CString name, XmlElement& out) const;

	ANKI_USE_RESULT Error getChildElementOptional(CString name, XmlElement& out) const;

private:
	tinyxml2::XMLDocument m_doc;
	GenericMemoryPoolAllocator<U8> m_alloc;
};
/// @}

} // end namespace anki

#include <anki/util/Xml.inl.h>
