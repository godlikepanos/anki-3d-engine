// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/List.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup util_other
/// @{

/// Define a global CVAR.
#define ANKI_CVAR(type, namespace, name, ...) inline type g_cvar##namespace##name(ANKI_STRINGIZE(namespace) "." ANKI_STRINGIZE(name), __VA_ARGS__);

/// Same as ANKI_CVAR but you can define 2 namespaces.
#define ANKI_CVAR2(type, namespaceA, namespaceB, name, ...) \
	inline type g_cvar##namespaceA##namespaceB##name(ANKI_STRINGIZE(namespaceA) "." ANKI_STRINGIZE(namespaceB) "." ANKI_STRINGIZE(name), __VA_ARGS__);

enum class CVarValueType : U32
{
	kString,
	kBool,
	kNumericU8,
	kNumericU16,
	kNumericU32,
	kNumericPtrSize,
	kNumericF32,
	kNumericF64
};

/// ConfigSet variable base.
class CVar : public IntrusiveListEnabled<CVar>
{
	friend class CVarSet;

public:
	CVar(const CVar&) = delete;

	CVar& operator=(const CVar&) = delete;

	CString getName() const
	{
		return m_name;
	}

	CVarValueType getValueType() const
	{
		return m_type;
	}

protected:
	CString m_name;
	CString m_descr;

	CVarValueType m_type;

	CVar(CVarValueType type, CString name, CString descr = {})
		: m_name(name)
		, m_descr((!descr.isEmpty()) ? descr : "No description")
		, m_type(type)
	{
		registerSelf();
	}

private:
	void registerSelf();
};

/// Numeric config variable.
template<typename TNumber>
class NumericCVar : public CVar
{
public:
	using CheckValueCallback = Bool (*)(TNumber);

	/// Initialize using a custom callback the checks the correctness of the value.
	NumericCVar(CString name, TNumber defaultVal, CheckValueCallback checkValueCallback, CString descr = CString())
		: CVar(getCVarType(), name, descr)
		, m_value(defaultVal)
		, m_checkValueCallback(checkValueCallback)
	{
		ANKI_ASSERT(checkValueCallback);
		ANKI_ASSERT(checkValueCallback(defaultVal));
	}

	/// Initialize using a min max range.
	NumericCVar(CString name, TNumber defaultVal, TNumber min = getMinNumericLimit<TNumber>(), TNumber max = getMaxNumericLimit<TNumber>(),
				CString descr = CString())
		: CVar(getCVarType(), name, descr)
		, m_value(defaultVal)
		, m_min(min)
		, m_max(max)
	{
		ANKI_ASSERT(min <= max);
		ANKI_ASSERT(defaultVal >= min && defaultVal <= max);
	}

	NumericCVar& operator=(TNumber val)
	{
		Bool ok = true;
		if(!m_checkValueCallback)
		{
			const TNumber newVal = clamp(val, m_min, m_max);
			ok = (newVal == val);
			m_value = newVal;
		}
		else
		{
			ok = m_checkValueCallback(val);
			if(ok)
			{
				m_value = val;
			}
		}

		if(!ok)
		{
			ANKI_UTIL_LOGW("Wrong value set for config var: %s", m_name.cstr());
		}

		return *this;
	}

	operator TNumber() const
	{
		return m_value;
	}

private:
	TNumber m_value;
	TNumber m_min = getMinNumericLimit<TNumber>();
	TNumber m_max = getMaxNumericLimit<TNumber>();
	CheckValueCallback m_checkValueCallback = nullptr;

	static CVarValueType getCVarType();
};

#define ANKI_CVAR_NUMERIC_TYPE(type) \
	template<> \
	inline CVarValueType NumericCVar<type>::getCVarType() \
	{ \
		return CVarValueType::kNumeric##type; \
	}

ANKI_CVAR_NUMERIC_TYPE(U8)
ANKI_CVAR_NUMERIC_TYPE(U16)
ANKI_CVAR_NUMERIC_TYPE(U32)
ANKI_CVAR_NUMERIC_TYPE(PtrSize)
ANKI_CVAR_NUMERIC_TYPE(F32)
ANKI_CVAR_NUMERIC_TYPE(F64)
#undef ANKI_CVAR_NUMERIC_TYPE

/// String config variable.
class StringCVar : public CVar
{
public:
	StringCVar(CString name, CString value, CString descr = CString())
		: CVar(CVarValueType::kString, name, descr)
	{
		*this = value;
	}

	~StringCVar()
	{
		if(m_str)
		{
			free(m_str);
		}
	}

	StringCVar& operator=(CString name)
	{
		if(m_str)
		{
			free(m_str);
		}
		const U len = name.getLength();
		m_str = static_cast<Char*>(malloc(len + 1));

		if(len == 0)
		{
			m_str[0] = '\0';
		}
		else
		{
			memcpy(m_str, name.cstr(), len + 1);
		}

		return *this;
	}

	operator CString() const
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
	explicit BoolCVar(CString name, Bool defaultVal, CString descr = CString())
		: CVar(CVarValueType::kBool, name, descr)
		, m_val(defaultVal)
	{
	}

	BoolCVar& operator=(Bool val)
	{
		m_val = val;
		return *this;
	}

	operator Bool() const
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

	Error setFromCommandLineArguments(U32 cmdLineArgsCount, Char* cmdLineArgs[])
	{
		ConstWeakArray<const Char*> arr(cmdLineArgs, cmdLineArgsCount);
		return setMultiple(arr);
	}

	template<U32 kTCount>
	Error setMultiple(Array<const Char*, kTCount> arr)
	{
		return setMultiple(ConstWeakArray<const Char*>(arr.getBegin(), kTCount));
	}

	Error setMultiple(ConstWeakArray<const Char*> arr);

	template<typename TFunc>
	void iterateCVars(TFunc func)
	{
		for(CVar& cvar : m_cvars)
		{
			if(func(cvar) == FunctorContinue::kStop)
			{
				return;
			}
		}
	}

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
