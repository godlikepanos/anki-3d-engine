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
	void set(CString optionName, CString value);

	template<typename T, ANKI_ENABLE(std::is_integral<T>::value)>
	void set(CString optionName, T value)
	{
		setInternal(optionName, U64(value));
	}

	template<typename T, ANKI_ENABLE(std::is_floating_point<T>::value)>
	void set(CString optionName, T value)
	{
		setInternal(optionName, F64(value));
	}
	/// @}

	/// @name Find an option and return its value.
	/// @{
	F64 getNumberF64(CString optionName) const;
	F32 getNumberF32(CString optionName) const;
	U64 getNumberU64(CString optionName) const;
	U32 getNumberU32(CString optionName) const;
	U16 getNumberU16(CString optionName) const;
	U8 getNumberU8(CString optionName) const;
	Bool getBool(CString optionName) const;
	CString getString(CString optionName) const;
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

	void setInternal(CString optionName, F64 value);
	void setInternal(CString optionName, U64 value);

	void newOptionInternal(CString optionName, U64 value, U64 minValue, U64 maxValue, CString helpMsg);
	void newOptionInternal(CString optionName, F64 value, F64 minValue, F64 maxValue, CString helpMsg);
};

/// The default config set. Copy that to your own to override.
using DefaultConfigSet = Singleton<ConfigSet>;

/// This macro registers a config option. Have it on .cpp files.
#define ANKI_REGISTER_CONFIG_OPTION(name, ...) \
	struct ANKI_CONCATENATE(ConfigSet, name) \
	{ \
		ANKI_CONCATENATE(ConfigSet, name)() \
		{ \
			DefaultConfigSet::get().newOption(#name, __VA_ARGS__); \
		} \
	} ANKI_CONCATENATE(g_ConfigSet, name);
/// @}

} // end namespace anki
