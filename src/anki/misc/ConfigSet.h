// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/misc/Common.h>
#include <anki/util/List.h>
#include <anki/util/String.h>

namespace anki
{

/// @addtogroup misc
/// @{

/// A storage of configuration variables.
class ConfigSet
{
public:
	ConfigSet();

	/// Copy.
	ConfigSet(const ConfigSet& b)
	{
		operator=(b);
	}

	~ConfigSet();

	/// Copy.
	ConfigSet& operator=(const ConfigSet& b);

	/// @name Set an option
	/// @{
	void set(const CString& name, const CString& value);
	void set(const CString& name, F64 value);
	/// @}

	/// @name Find an option and return its value.
	/// @{
	F64 getNumberF64(const CString& name) const;
	F32 getNumberF32(const CString& name) const;
	U32 getNumberU32(const CString& name) const;
	U16 getNumberU16(const CString& name) const;
	U8 getNumberU8(const CString& name) const;
	Bool getBool(const CString& name) const;
	CString getString(const CString& name) const;
	/// @}

	ANKI_USE_RESULT Error loadFromFile(CString filename);

	ANKI_USE_RESULT Error saveToFile(CString filename) const;

	ANKI_USE_RESULT Error setFromCommandLineArguments(U cmdLineArgsCount, char* cmdLineArgs[]);

protected:
	void newOption(const CString& name, const CString& value, const CString& helpMsg = "");
	void newOption(const CString& name, F64 value, const CString& helpMsg = "");

private:
	class Option : public NonCopyable
	{
	public:
		String m_name;
		String m_strVal;
		String m_helpMsg;
		F64 m_fVal = 0.0f;
		U8 m_type = 0; ///< 0: string, 1: float

		Option() = default;

		Option(Option&& b)
			: m_name(std::move(b.m_name))
			, m_strVal(std::move(b.m_strVal))
			, m_helpMsg(std::move(b.m_helpMsg))
			, m_fVal(b.m_fVal)
			, m_type(b.m_type)
		{
		}

		~Option() = default;

		Option& operator=(Option&& b) = delete;
	};

	HeapAllocator<U8> m_alloc;
	List<Option> m_options;

	Option* tryFind(const CString& name);
	const Option* tryFind(const CString& name) const;
};
/// @}

} // end namespace anki
