// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Math/Functions.h>

namespace anki {

/// @addtogroup core
/// @{

enum class CVarSubsystem : U32
{
	kCore,
	kRenderer,
	kGr,
	kResource,
	kScene,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(CVarSubsystem)

/// Config variable base.
class CVar : public IntrusiveListEnabled<CVar>
{
	friend class CVarSet;

public:
	CVar(const CVar&) = delete;

	CVar& operator=(const CVar&) = delete;

	CoreString getFullName() const;

protected:
	CString m_name;
	CString m_descr;
	CVarSubsystem m_subsystem;

	enum class Type : U32
	{
		kString,
		kBool,
		kNumericU8,
		kNumericU32,
		kNumericPtrSize,
		kNumericF32
	};

	Type m_type;

	CVar(Type type, CVarSubsystem subsystem, CString name, CString descr = {})
		: m_name(name)
		, m_descr((!descr.isEmpty()) ? descr : "No description")
		, m_subsystem(subsystem)
		, m_type(type)
	{
		ANKI_ASSERT(subsystem < CVarSubsystem::kCount);
		registerSelf();
	}

private:
	void registerSelf();

	void getFullNameInternal(Array<Char, 256>& arr) const;
};

/// Numeric config variable.
template<typename TNumber>
class NumericCVar : public CVar
{
public:
	NumericCVar(CVarSubsystem subsystem, CString name, TNumber defaultVal, TNumber min = getMinNumericLimit<TNumber>(),
				TNumber max = getMaxNumericLimit<TNumber>(), CString descr = CString())
		: CVar(getCVarType(), subsystem, name, descr)
		, m_value(defaultVal)
		, m_min(min)
		, m_max(max)
	{
		ANKI_ASSERT(min <= max);
		ANKI_ASSERT(defaultVal >= min && defaultVal <= max);
	}

	void set(TNumber val)
	{
		const TNumber newVal = clamp(val, m_min, m_max);
		if(newVal != val)
		{
			ANKI_CORE_LOGW("Out of range value set for config var: %s", m_name.cstr());
		}
		m_value = newVal;
	}

	TNumber get() const
	{
		return m_value;
	}

private:
	TNumber m_value;
	TNumber m_min;
	TNumber m_max;

	static Type getCVarType();
};

#define ANKI_CVAR_NUMERIC_TYPE(type) \
	template<> \
	inline CVar::Type NumericCVar<type>::getCVarType() \
	{ \
		return Type::kNumeric##type; \
	}

ANKI_CVAR_NUMERIC_TYPE(U8)
ANKI_CVAR_NUMERIC_TYPE(U32)
ANKI_CVAR_NUMERIC_TYPE(PtrSize)
ANKI_CVAR_NUMERIC_TYPE(F32)
#undef ANKI_CVAR_NUMERIC_TYPE

/// String config variable.
class StringCVar : public CVar
{
public:
	StringCVar(CVarSubsystem subsystem, CString name, CString value, CString descr = CString())
		: CVar(Type::kString, subsystem, name, descr)
	{
		set(value);
	}

	~StringCVar()
	{
		if(m_str)
		{
			free(m_str);
		}
	}

	void set(CString name)
	{
		ANKI_ASSERT(name.getLength() > 0);
		if(m_str)
		{
			free(m_str);
		}
		const U len = name.getLength();
		m_str = static_cast<Char*>(malloc(len + 1));
		memcpy(m_str, name.cstr(), len + 1);
	}

	CString get() const
	{
		return m_str;
	}

private:
	Char* m_str;
};

/// Boolean config variable.
class BoolCVar : public CVar
{
public:
	BoolCVar(CVarSubsystem subsystem, CString name, Bool defaultVal, CString descr = CString())
		: CVar(Type::kBool, subsystem, name, descr)
		, m_val(defaultVal)
	{
	}

	void set(Bool val)
	{
		m_val = val;
	}

	Bool get() const
	{
		return m_val;
	}

private:
	Bool m_val;
};

/// Access all configuration variables.
class CVarSet : public MakeSingletonLazyInit<CVarSet>
{
	friend class CVar;

public:
	CVarSet() = default;

	CVarSet(const CVarSet& b) = delete; // Non-copyable

	CVarSet& operator=(const CVarSet& b) = delete; // Non-copyable

	Error loadFromFile(CString filename);

	Error saveToFile(CString filename) const;

	Error setFromCommandLineArguments(U32 cmdLineArgsCount, char* cmdLineArgs[]);

private:
	IntrusiveList<CVar> m_cvars;

	void registerCVar(CVar* var);
};

inline void CVar::registerSelf()
{
	CVarSet::getSingleton().registerCVar(this);
}
/// @}

} // end namespace anki
