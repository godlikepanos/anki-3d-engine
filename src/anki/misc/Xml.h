// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/misc/Common.h>
#include <anki/util/String.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/StringList.h>
#include <anki/Math.h>
#include <tinyxml2.h>
#if !ANKI_TINYXML2
#	error "Wrong tinyxml2 included"
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

	/// Return the text inside a tag. May return empty string.
	ANKI_USE_RESULT Error getText(CString& out) const;

	/// Return the text inside as a number.
	template<typename T>
	ANKI_USE_RESULT Error getNumber(T& out) const
	{
		ANKI_CHECK(check());

		const char* txt = m_el->GetText();
		if(txt != nullptr)
		{
			ANKI_CHECK(CString(txt).toNumber(out));
		}
		else
		{
			ANKI_MISC_LOGE("Failed to return number. Element: %s", m_el->Value());
			return Error::USER_DATA;
		}

		return Error::NONE;
	}

	/// Get a number of numbers.
	template<typename T>
	ANKI_USE_RESULT Error getNumbers(DynamicArrayAuto<T>& out) const
	{
		CString txt;
		ANKI_CHECK(getText(txt));

		if(txt)
		{
			return parseNumbers(txt, out);
		}
		else
		{
			out = DynamicArrayAuto<T>(m_alloc);
			return Error::NONE;
		}
	}

	/// Return the text inside as a Mat4
	ANKI_USE_RESULT Error getMat4(Mat4& out) const;

	/// Return the text inside as a Mat3
	ANKI_USE_RESULT Error getMat3(Mat3& out) const;

	/// Return the text inside as a Vec2
	ANKI_USE_RESULT Error getVec2(Vec2& out) const;

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

	/// @name Get attributes optional
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	ANKI_USE_RESULT Error getAttributeTextOptional(const CString& name, CString& out, Bool& attribPresent) const;

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumbersOptional(
		const CString& name, DynamicArrayAuto<T>& out, Bool& attribPresent) const
	{
		CString txtVal;
		ANKI_CHECK(getAttributeTextOptional(name, txtVal, attribPresent));

		if(txtVal && attribPresent)
		{
			return parseNumbers(txtVal, out);
		}
		else
		{
			return Error::NONE;
		}
	}

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumberOptional(const CString& name, T& out, Bool& attribPresent) const
	{
		DynamicArrayAuto<T> arr(m_alloc);
		ANKI_CHECK(getAttributeNumbersOptional(name, arr, attribPresent));

		if(attribPresent)
		{
			if(arr.getSize() != 1)
			{
				ANKI_MISC_LOGE("Expecting one element for attrib %s", &name[0]);
				return Error::USER_DATA;
			}

			out = arr[0];
		}

		return Error::NONE;
	}

	/// Get the attribute's value as a vector.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeVectorOptional(const CString& name, T& out, Bool& attribPresent) const
	{
		DynamicArrayAuto<F32> arr(m_alloc);
		ANKI_CHECK(getAttributeNumbersOptional(name, arr, attribPresent));

		if(attribPresent)
		{
			if(arr.getSize() != sizeof(T) / sizeof(out[0]))
			{
				ANKI_MISC_LOGE("Expecting %u elements for attrib %s", sizeof(T) / sizeof(out[0]), &name[0]);
				return Error::USER_DATA;
			}

			U count = 0;
			for(F32 v : arr)
			{
				out[count++] = v;
			}
		}

		return Error::NONE;
	}

	/// Get the attribute's value as a matrix.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	/// @param attribPresent True if the attribute exists. If it doesn't the @a out is undefined.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeMatrixOptional(const CString& name, T& out, Bool& attribPresent) const
	{
		return getAttributeVectorOptional(name, out, attribPresent);
	}
	/// @}

	/// @name Get attributes
	/// @{

	/// Get value of a string attribute. May return empty string.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	ANKI_USE_RESULT Error getAttributeText(const CString& name, CString& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeTextOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a series of numbers.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumbers(const CString& name, DynamicArrayAuto<T>& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumbersOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a number.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeNumber(const CString& name, T& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeNumberOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a vector.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeVector(const CString& name, T& out) const
	{
		Bool found;
		ANKI_CHECK(getAttributeVectorOptional(name, out, found));
		return throwAttribNotFoundError(name, found);
	}

	/// Get the attribute's value as a matrix.
	/// @param name The name of the attribute.
	/// @param out The value of the attribute.
	template<typename T>
	ANKI_USE_RESULT Error getAttributeMatrix(const CString& name, T& out) const
	{
		return getAttributeVector(name, out);
	}
	/// @}

private:
	const tinyxml2::XMLElement* m_el;
	GenericMemoryPoolAllocator<U8> m_alloc;

	ANKI_USE_RESULT Error check() const;

	template<typename T>
	ANKI_USE_RESULT Error parseNumbers(CString txt, DynamicArrayAuto<T>& out) const
	{
		ANKI_ASSERT(txt);
		ANKI_ASSERT(m_el);

		StringListAuto list(m_alloc);
		list.splitString(txt, ' ');

		out = DynamicArrayAuto<T>(m_alloc);
		out.create(list.getSize());

		Error err = Error::NONE;
		auto it = list.getBegin();
		auto end = list.getEnd();
		U i = 0;
		while(it != end && !err)
		{
			err = it->toNumber(out[i++]);
			++it;
		}

		if(err)
		{
			ANKI_MISC_LOGE("Failed to parse floats. Element: %s", m_el->Value());
		}

		return err;
	}

	ANKI_USE_RESULT Error throwAttribNotFoundError(CString attrib, Bool found) const
	{
		if(!found)
		{
			ANKI_MISC_LOGE("Attribute not found \"%s\"", &attrib[0]);
			return Error::USER_DATA;
		}
		else
		{
			return Error::NONE;
		}
	}
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
