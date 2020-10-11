// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/script/Common.h>
#include <anki/util/Assert.h>
#include <anki/util/StdTypes.h>
#include <anki/util/Allocator.h>
#include <anki/util/String.h>
#include <anki/util/Functions.h>
#include <anki/util/HashMap.h>
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "Wrong LUA header included"
#endif

namespace anki
{

// Forward
class LuaUserData;
class SceneGraph;
class MainRenderer;

/// @addtogroup script
/// @{

/// @memberof LuaUserData
using LuaUserDataSerializeCallback = void (*)(LuaUserData& self, void* data, PtrSize& size);

/// @memberof LuaUserData
using LuaUserDataDeserializeCallback = void (*)(const void* data, LuaUserData& self);

/// @memberof LuaUserData
class LuaUserDataTypeInfo
{
public:
	I64 m_signature;
	const char* m_typeName;
	PtrSize m_structureSize;
	LuaUserDataSerializeCallback m_serializeCallback;
	LuaUserDataDeserializeCallback m_deserializeCallback;
};

/// LUA userdata.
class LuaUserData
{
public:
	/// @note NEVER ADD A DESTRUCTOR. LUA cannot call that.
	~LuaUserData() = delete;

	I64 getSig() const
	{
		return m_sig;
	}

	void initGarbageCollected(const LuaUserDataTypeInfo* info)
	{
		ANKI_ASSERT(info);
		m_sig = info->m_signature;
		m_info = info;
		m_addressOrGarbageCollect = GC_MASK;
	}

	void initPointed(const LuaUserDataTypeInfo* info, void* ptrToObject)
	{
		ANKI_ASSERT(info);
		m_sig = info->m_signature;
		m_info = info;
		U64 addr = ptrToNumber(ptrToObject);
		ANKI_ASSERT((addr & GC_MASK) == 0 && "Address too high, cannot encode a flag");
		m_addressOrGarbageCollect = addr;
	}

	Bool isGarbageCollected() const
	{
		ANKI_ASSERT(m_addressOrGarbageCollect != 0);
		return m_addressOrGarbageCollect == GC_MASK;
	}

	template<typename T>
	T* getData()
	{
		ANKI_ASSERT(m_addressOrGarbageCollect != 0);
		ANKI_ASSERT(getDataTypeInfoFor<T>().m_signature == m_sig);
		T* out = nullptr;
		if(isGarbageCollected())
		{
			// Garbage collected, the data -in memory- are after this object
			PtrSize mem = ptrToNumber(this);
			mem += getAlignedRoundUp(alignof(T), sizeof(LuaUserData));
			out = numberToPtr<T*>(mem);
		}
		else
		{
			// Pointed
			PtrSize mem = static_cast<PtrSize>(m_addressOrGarbageCollect);
			out = numberToPtr<T*>(mem);
		}

		ANKI_ASSERT(out);
		ANKI_ASSERT(isAligned(alignof(T), out));
		return out;
	}

	template<typename T>
	static PtrSize computeSizeForGarbageCollected()
	{
		return getAlignedRoundUp(alignof(T), sizeof(LuaUserData)) + sizeof(T);
	}

	const LuaUserDataTypeInfo& getDataTypeInfo() const
	{
		ANKI_ASSERT(m_info);
		return *m_info;
	}

	template<typename TWrapedType>
	static const LuaUserDataTypeInfo& getDataTypeInfoFor();

private:
	static constexpr U64 GC_MASK = U64(1) << U64(63);

	I64 m_sig = 0; ///< Signature to identify the user data.

	U64 m_addressOrGarbageCollect = 0; ///< Encodes an address or a flag if it's for garbage collection.

	const LuaUserDataTypeInfo* m_info = nullptr;
};

/// @memberof LuaBinder
class LuaBinderSerializeGlobalsCallback
{
public:
	virtual void write(const void* data, PtrSize dataSize) = 0;
};

/// A list of systems that the LuaBinder should be aware of.
/// @memberof LuaBinder
class LuaBinderOtherSystems
{
public:
	SceneGraph* m_sceneGraph;
	MainRenderer* m_renderer;
};

/// Lua binder class. A wrapper on top of LUA
class LuaBinder : public NonCopyable
{
public:
	LuaBinder();
	~LuaBinder();

	ANKI_USE_RESULT Error init(ScriptAllocator alloc, LuaBinderOtherSystems* otherSystems);

	lua_State* getLuaState()
	{
		ANKI_ASSERT(m_l);
		return m_l;
	}

	ScriptAllocator getAllocator() const
	{
		return m_alloc;
	}

	LuaBinderOtherSystems& getOtherSystems()
	{
		ANKI_ASSERT(m_otherSystems);
		return *m_otherSystems;
	}

	/// Expose a variable to the lua state
	template<typename T>
	static void exposeVariable(lua_State* state, CString name, T* y)
	{
		void* ptr = lua_newuserdata(state, sizeof(LuaUserData));
		LuaUserData* ud = static_cast<LuaUserData*>(ptr);
		ud->initPointed(&LuaUserData::getDataTypeInfoFor<T>(), y);
		luaL_setmetatable(state, LuaUserData::getDataTypeInfoFor<T>().m_typeName);
		lua_setglobal(state, name.cstr());
	}

	template<typename T>
	static void pushVariableToTheStack(lua_State* state, T* y)
	{
		void* ptr = lua_newuserdata(state, sizeof(LuaUserData));
		LuaUserData* ud = static_cast<LuaUserData*>(ptr);
		ud->initPointed(&LuaUserData::getDataTypeInfoFor<T>(), y);
		luaL_setmetatable(state, LuaUserData::getDataTypeInfoFor<T>().m_typeName);
	}

	/// Evaluate a string
	static Error evalString(lua_State* state, const CString& str);

	static void garbageCollect(lua_State* state)
	{
		lua_gc(state, LUA_GCCOLLECT, 0);
	}

	/// For debugging purposes
	static void stackDump(lua_State* l);

	/// Create a new LUA class
	static void createClass(lua_State* l, const LuaUserDataTypeInfo* typeInfo);

	/// Add new function in a class that it's already in the stack
	static void pushLuaCFuncMethod(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Add a new static function in the class.
	static void pushLuaCFuncStaticMethod(lua_State* l, const char* className, const char* name, lua_CFunction luafunc);

	/// Add a new function.
	static void pushLuaCFunc(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Dump global variables.
	static void serializeGlobals(lua_State* l, LuaBinderSerializeGlobalsCallback& callback);

	/// Deserialize global variables.
	static void deserializeGlobals(lua_State* l, const void* data, PtrSize dataSize);

	/// Make sure that the arguments match the argsCount number
	static ANKI_USE_RESULT Error checkArgsCount(lua_State* l, I argsCount);

	/// Get a number from the stack.
	template<typename TNumber>
	static ANKI_USE_RESULT Error checkNumber(lua_State* l, I32 stackIdx, TNumber& number)
	{
		lua_Number lnum;
		Error err = checkNumberInternal(l, stackIdx, lnum);
		if(!err)
		{
			number = TNumber(lnum);
		}
		return err;
	}

	/// Get a string from the stack.
	static ANKI_USE_RESULT Error checkString(lua_State* l, I32 stackIdx, const char*& out);

	/// Get some user data from the stack.
	/// The function uses the type signature to validate the type and not the
	/// typeName. That is supposed to be faster.
	static ANKI_USE_RESULT Error checkUserData(lua_State* l, I32 stackIdx, const LuaUserDataTypeInfo& typeInfo,
											   LuaUserData*& out);

	/// Allocate memory.
	static void* luaAlloc(lua_State* l, size_t size, U32 alignment);

	/// Free memory.
	static void luaFree(lua_State* l, void* ptr);

private:
	LuaBinderOtherSystems* m_otherSystems;
	ScriptAllocator m_alloc;
	lua_State* m_l = nullptr;
	HashMap<I64, const LuaUserDataTypeInfo*> m_userDataSigToDataInfo;

	static void* luaAllocCallback(void* userData, void* ptr, PtrSize osize, PtrSize nsize);

	static ANKI_USE_RESULT Error checkNumberInternal(lua_State* l, I32 stackIdx, lua_Number& number);
};
/// @}

} // end namespace anki
