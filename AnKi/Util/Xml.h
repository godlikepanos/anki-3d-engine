// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/String.h>
#include <TinyXml2/include/tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
#endif

namespace anki {

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
		, m_pool(b.m_pool)
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
		m_pool = b.m_pool;
		return *this;
	}

	/// Return the text inside a tag. May return empty string.
	Error getText(CString& out) const;

	/// Return the text inside as a number.
	template<typename T>
	Error getNumber(T& out) const;

	/// Get a number of numbers.
	template<typename T>
	Error getNumbers(DynamicArrayRaii<T>& out) const;

	/// Get a fixed number of numbers.
	/// @tparam TArray A type that should have operator[] and getSize() methods implemented.
	template<typename TArray>
	Error getNumbers(TArray& out) const;

	/// Get a child element quietly
	Error getChildElementOptional(CString name, XmlElement& out) const;

	/// Get a child element and print error if not found
	Error getChildElement(CString name, XmlElement& out) const;

	/// Get the next element with the same name. Returns empty XmlElement if it reached the end of the list.
	Error getNextSiblingElement(CString name, XmlElement& out) const;

	/// Get the number of sibling elements of this node.
	/// @note The sibling elements share the same name.
	Error getSiblingElementsCount(U32& out) const;

	/// @name Get attributes optional
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	Error getAttributeTextOptional(CString name, CString& out, Bool& attribPresent) const;

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	Error getAttributeNumbersOptional(CString name, DynamicArrayRaii<T>& out, Bool& attribPresent) const;

	/// Get the attribute's value as a series of numbers.
	/// @tparam TArray A type that should have operator[] and getSize() methods implemented.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename TArray>
	Error getAttributeNumbersOptional(CString name, TArray& out, Bool& attribPresent) const;

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	Error getAttributeNumberOptional(CString name, T& out, Bool& attribPresent) const;
	/// @}

	/// @name Get attributes
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	Error getAttributeText(CString name, CString& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeTextOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	Error getAttributeNumbers(CString name, DynamicArrayRaii<T>& out) const
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
	Error getAttributeNumbers(CString name, TArray& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumbersOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	Error getAttributeNumber(CString name, T& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumberOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}
	/// @}

private:
	const tinyxml2::XMLElement* m_el;
	BaseMemoryPool* m_pool = nullptr;

	XmlElement(const tinyxml2::XMLElement* el, BaseMemoryPool* pool)
		: m_el(el)
		, m_pool(pool)
	{
	}

	Error check() const;

	template<typename T>
	Error parseNumbers(CString txt, DynamicArrayRaii<T>& out) const;

	template<typename TArray>
	Error parseNumbers(CString txt, TArray& out) const;

	Error throwAttribNotFoundError(CString attrib, Bool found) const
	{
		if(!found)
		{
			ANKI_UTIL_LOGE("Attribute not found \"%s\"", &attrib[0]);
			return Error::kUserData;
		}
		else
		{
			return Error::kNone;
		}
	}
};

/// XML document.
class XmlDocument
{
public:
	static constexpr CString kXmlHeader = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

	XmlDocument(BaseMemoryPool* pool)
		: m_pool(pool)
	{
	}

	/// Parse from a file.
	Error loadFile(CString filename);

	/// Parse from a CString.
	Error parse(CString xmlText);

	Error getChildElement(CString name, XmlElement& out) const;

	Error getChildElementOptional(CString name, XmlElement& out) const;

	BaseMemoryPool& getMemoryPool()
	{
		return *m_pool;
	}

private:
	tinyxml2::XMLDocument m_doc;
	BaseMemoryPool* m_pool = nullptr;
};
/// @}

} // end namespace anki

#include <AnKi/Util/Xml.inl.h>
