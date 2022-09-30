// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/String.h>
#include <AnKi/Math/Functions.h>

namespace anki {

/// @addtogroup core
/// @{

/// A storage of configuration variables.
class ConfigSet
{
public:
	/// @note Need to call init() if using this constructor.
	ConfigSet()
	{
	}

	ConfigSet(AllocAlignedCallback allocCb, void* allocCbUserData)
	{
		init(allocCb, allocCbUserData);
	}

	ConfigSet(const ConfigSet& b) = delete; // Non-copyable

	~ConfigSet();

	ConfigSet& operator=(const ConfigSet& b) = delete; // Non-copyable

	void init(AllocAlignedCallback allocCb, void* allocCbUserData);

	Error loadFromFile(CString filename);

	Error saveToFile(CString filename) const;

	Error setFromCommandLineArguments(U32 cmdLineArgsCount, char* cmdLineArgs[]);

	// Define getters and setters
#if ANKI_EXTRA_CHECKS
#	define ANKI_CONFIG_VAR_GET(type, name, defaultValue, minValue, maxValue, description) \
		type get##name() const \
		{ \
			ANKI_ASSERT(isInitialized()); \
			m_##name.m_timesAccessed.fetchAdd(1); \
			return m_##name.m_value; \
		}
#else
#	define ANKI_CONFIG_VAR_GET(type, name, defaultValue, minValue, maxValue, description) \
		type get##name() const \
		{ \
			ANKI_ASSERT(isInitialized()); \
			return m_##name.m_value; \
		}
#endif
#define ANKI_CONFIG_VAR_SET(type, name, defaultValue, minValue, maxValue, description) \
	void set##name(type value) \
	{ \
		ANKI_ASSERT(isInitialized()); \
		const type newValue = clamp(value, m_##name.m_min, m_##name.m_max); \
		if(newValue != value) \
		{ \
			ANKI_CORE_LOGW("Out of range value set for config var: %s", m_##name.m_name.cstr()); \
		} \
		m_##name.m_value = newValue; \
	}
#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_GET(U8, name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_SET(U8, name, defaultValue, minValue, maxValue, description)
#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_GET(U32, name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_SET(U32, name, defaultValue, minValue, maxValue, description)
#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_GET(PtrSize, name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_SET(PtrSize, name, defaultValue, minValue, maxValue, description)
#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_GET(F32, name, defaultValue, minValue, maxValue, description) \
	ANKI_CONFIG_VAR_SET(F32, name, defaultValue, minValue, maxValue, description)
#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) \
	ANKI_CONFIG_VAR_GET(Bool, name, defaultValue, minValue, maxValue, description) \
	void set##name(Bool value) \
	{ \
		ANKI_ASSERT(isInitialized()); \
		m_##name.m_value = value; \
	}
#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) \
	ANKI_CONFIG_VAR_GET(CString, name, defaultValue, minValue, maxValue, description) \
	void set##name(CString value) \
	{ \
		ANKI_ASSERT(isInitialized()); \
		setStringVar(value, m_##name); \
	}
#include <AnKi/Core/AllConfigVars.defs.h>
#undef ANKI_CONFIG_VAR_GET
#undef ANKI_CONFIG_VAR_SET

private:
	class Var : public IntrusiveListEnabled<Var>
	{
	public:
		CString m_name;
		CString m_description;
#if ANKI_EXTRA_CHECKS
		mutable Atomic<U32> m_timesAccessed = {0};
#endif
	};

	template<typename T>
	class NumberVar : public Var
	{
	public:
		T m_value;
		T m_min;
		T m_max;
	};

	class StringVar : public Var
	{
	public:
		Char* m_value = nullptr;
	};

	class BoolVar : public Var
	{
	public:
		Bool m_value;
	};

	HeapMemoryPool m_pool;
	IntrusiveList<Var> m_vars;

#define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description) NumberVar<U8> m_##name;
#define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description) NumberVar<U32> m_##name;
#define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description) NumberVar<PtrSize> m_##name;
#define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description) NumberVar<F32> m_##name;
#define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description) BoolVar m_##name;
#define ANKI_CONFIG_VAR_STRING(name, defaultValue, description) StringVar m_##name;
#include <AnKi/Core/AllConfigVars.defs.h>

	Bool isInitialized() const
	{
		return !m_vars.isEmpty();
	}

	Var* tryFind(CString name);
	const Var* tryFind(CString name) const;

	Var& find(CString name)
	{
		Var* o = tryFind(name);
		ANKI_ASSERT(o && "Couldn't find config option");
		return *o;
	}

	const Var& find(CString name) const
	{
		const Var* o = tryFind(name);
		ANKI_ASSERT(o && "Couldn't find config option");
		return *o;
	}

	void setStringVar(CString val, StringVar& var)
	{
		m_pool.free(var.m_value);
		const U32 len = val.getLength();
		var.m_value = static_cast<Char*>(m_pool.allocate(len + 1, alignof(Char)));
		memcpy(var.m_value, val.getBegin(), len + 1);
	}
};
/// @}

} // end namespace anki
