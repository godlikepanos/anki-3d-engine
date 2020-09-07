// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/List.h>
#include <anki/util/String.h>

namespace anki
{

/// @addtogroup core
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

	/// @name Set the value of an option.
	/// @{
	void set(CString option, CString value);

	template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
	void set(CString option, T value)
	{
		setInternal(option, U64(value));
	}

	template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
	void set(CString option, T value)
	{
		setInternal(option, F64(value));
	}
	/// @}

	/// @name Find an option and return its value.
	/// @{
	F64 getNumberF64(CString option) const;
	F32 getNumberF32(CString option) const;
	U64 getNumberU64(CString option) const;
	U32 getNumberU32(CString option) const;
	U16 getNumberU16(CString option) const;
	U8 getNumberU8(CString option) const;
	Bool getBool(CString option) const;
	CString getString(CString option) const;
	/// @}

	/// @name Create new options.
	/// @{
	void newOption(CString optionName, CString value, CString helpMsg);

	template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
	void newOption(CString optionName, T value, T minValue, T maxValue, CString helpMsg = "")
	{
		newOptionInternal(optionName, U64(value), U64(minValue), U64(maxValue), helpMsg);
	}

	template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
	void newOption(CString optionName, T value, T minValue, T maxValue, CString helpMsg = "")
	{
		newOptionInternal(optionName, F64(value), F64(minValue), F64(maxValue), helpMsg);
	}
	/// @}

	ANKI_USE_RESULT Error loadFromFile(CString filename);

	ANKI_USE_RESULT Error saveToFile(CString filename) const;

	ANKI_USE_RESULT Error setFromCommandLineArguments(U32 cmdLineArgsCount, char* cmdLineArgs[]);

private:
	class Option;

	HeapAllocator<U8> m_alloc;
	List<Option> m_options;

	Option* tryFind(CString name);
	const Option* tryFind(CString name) const;

	Option& find(CString name)
	{
		Option* o = tryFind(name);
		ANKI_ASSERT(o && "Couldn't find config option");
		return *o;
	}

	const Option& find(CString name) const
	{
		const Option* o = tryFind(name);
		ANKI_ASSERT(o && "Couldn't find config option");
		return *o;
	}

	void setInternal(CString option, F64 value);
	void setInternal(CString option, U64 value);

	void newOptionInternal(CString optionName, U64 value, U64 minValue, U64 maxValue, CString helpMsg);
	void newOptionInternal(CString optionName, F64 value, F64 minValue, F64 maxValue, CString helpMsg);
};

/// The default config set. Copy that to your own to override.
using DefaultConfigSet = Singleton<ConfigSet>;
/// @}

} // end namespace anki
