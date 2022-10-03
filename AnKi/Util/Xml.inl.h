// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Xml.h>
#include <AnKi/Util/StringList.h>

namespace anki {

template<typename T>
Error XmlElement::getNumber(T& out) const
{
	ANKI_CHECK(check());

	const char* txt = m_el->GetText();
	if(txt != nullptr)
	{
		ANKI_CHECK(CString(txt).toNumber(out));
	}
	else
	{
		ANKI_UTIL_LOGE("Failed to return number. Element: %s", m_el->Value());
		return Error::kUserData;
	}

	return Error::kNone;
}

template<typename T>
Error XmlElement::getNumbers(DynamicArrayRaii<T>& out) const
{
	CString txt;
	ANKI_CHECK(getText(txt));

	if(txt)
	{
		return parseNumbers(txt, out);
	}
	else
	{
		out.destroy();
		return Error::kNone;
	}
}

template<typename TArray>
Error XmlElement::getNumbers(TArray& out) const
{
	CString txt;
	ANKI_CHECK(getText(txt));
	return parseNumbers(txt, out);
}

template<typename T>
Error XmlElement::getAttributeNumbersOptional(CString name, DynamicArrayRaii<T>& out, Bool& attribPresent) const
{
	CString txtVal;
	ANKI_CHECK(getAttributeTextOptional(name, txtVal, attribPresent));

	if(txtVal && attribPresent)
	{
		return parseNumbers(txtVal, out);
	}
	else
	{
		return Error::kNone;
	}
}

template<typename TArray>
Error XmlElement::getAttributeNumbersOptional(CString name, TArray& out, Bool& attribPresent) const
{
	CString txtVal;
	ANKI_CHECK(getAttributeTextOptional(name, txtVal, attribPresent));

	if(txtVal && attribPresent)
	{
		return parseNumbers(txtVal, out);
	}
	else
	{
		return Error::kNone;
	}
}

template<typename T>
Error XmlElement::getAttributeNumberOptional(CString name, T& out, Bool& attribPresent) const
{
	DynamicArrayRaii<T> arr(m_pool);
	ANKI_CHECK(getAttributeNumbersOptional(name, arr, attribPresent));

	if(attribPresent)
	{
		if(arr.getSize() != 1)
		{
			ANKI_UTIL_LOGE("Expecting one element for attrib: %s", &name[0]);
			return Error::kUserData;
		}

		out = arr[0];
	}

	return Error::kNone;
}

template<typename T>
Error XmlElement::parseNumbers(CString txt, DynamicArrayRaii<T>& out) const
{
	ANKI_ASSERT(txt);
	ANKI_ASSERT(m_el);

	StringListRaii list(m_pool);
	list.splitString(txt, ' ');

	out.destroy();
	out.create(U32(list.getSize()));

	Error err = Error::kNone;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U32 i = 0;
	while(it != end && !err)
	{
		err = it->toNumber(out[i++]);
		++it;
	}

	if(err)
	{
		ANKI_UTIL_LOGE("Failed to covert to numbers the element: %s", m_el->Value());
	}

	return err;
}

template<typename TArray>
Error XmlElement::parseNumbers(CString txt, TArray& out) const
{
	ANKI_ASSERT(!txt.isEmpty());
	ANKI_ASSERT(m_el);

	StringListRaii list(m_pool);
	list.splitString(txt, ' ');
	const PtrSize listSize = list.getSize();

	if(listSize != out.getSize())
	{
		ANKI_UTIL_LOGE("Wrong number of elements for element: %s", m_el->Value());
		return Error::kUserData;
	}

	Error err = Error::kNone;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U32 i = 0;
	while(it != end && !err)
	{
		err = it->toNumber(out[i++]);
		++it;
	}

	if(err)
	{
		ANKI_UTIL_LOGE("Failed to covert to numbers the element: %s", m_el->Value());
	}

	return err;
}

} // end namespace anki
