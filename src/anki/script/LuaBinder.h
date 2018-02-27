// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "Wrong LUA header included"
#endif

namespace anki
{

/// @addtogroup script
/// @{

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

	void initGarbageCollected(I64 sig)
	{
		m_sig = sig;
		m_addressOrGarbageCollect = GC_MASK;
	}

	void initPointed(I64 sig, void* ptrToObject)
	{
		m_sig = sig;
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

private:
	static constexpr U64 GC_MASK = U64(1) << U64(63);

	I64 m_sig = 0; ///< Signature to identify the user data.

	U64 m_addressOrGarbageCollect = 0; ///< Encodes an address or a flag if it's for garbage collection.
};

/// An instance of the original lua state with its own state.
class LuaThread : public NonCopyable
{
	friend class LuaBinder;

public:
	lua_State* m_luaState = nullptr;

	LuaThread() = default;

	LuaThread(LuaThread&& b)
	{
		*this = std::move(b);
	}

	~LuaThread()
	{
		ANKI_ASSERT(m_luaState == nullptr && "Forgot to deleteLuaThread");
	}

	LuaThread& operator=(LuaThread&& b)
	{
		ANKI_ASSERT(m_luaState == nullptr);
		m_luaState = b.m_luaState;
		b.m_luaState = nullptr;

		m_reference = b.m_reference;
		b.m_reference = -1;
		return *this;
	}

private:
	int m_reference = -1;
};

/// Lua binder class. A wrapper on top of LUA
class LuaBinder : public NonCopyable
{
public:
	LuaBinder();
	~LuaBinder();

	ANKI_USE_RESULT Error create(ScriptAllocator alloc, void* parent);

	lua_State* getLuaState()
	{
		return m_l;
	}

	ScriptAllocator getAllocator() const
	{
		return m_alloc;
	}

	void* getParent() const
	{
		return m_parent;
	}

	/// Expose a variable to the lua state
	template<typename T>
	static void exposeVariable(lua_State* state, CString name, T* y)
	{
		void* ptr = lua_newuserdata(state, sizeof(LuaUserData));
		LuaUserData* ud = static_cast<LuaUserData*>(ptr);
		ud->initPointed(getWrappedTypeSignature<T>(), y);
		luaL_setmetatable(state, getWrappedTypeName<T>());
		lua_setglobal(state, name.cstr());
	}

	template<typename T>
	static void pushVariableToTheStack(lua_State* state, T* y)
	{
		void* ptr = lua_newuserdata(state, sizeof(LuaUserData));
		LuaUserData* ud = static_cast<LuaUserData*>(ptr);
		ud->initPointed(getWrappedTypeSignature<T>(), y);
		luaL_setmetatable(state, getWrappedTypeName<T>());
	}

	/// Evaluate a string
	static Error evalString(lua_State* state, const CString& str);

	static void garbageCollect(lua_State* state)
	{
		lua_gc(state, LUA_GCCOLLECT, 0);
	}

	/// New LuaThread.
	LuaThread newLuaThread();

	/// Destroy a LuaThread.
	void destroyLuaThread(LuaThread& luaThread);

	/// For debugging purposes
	static void stackDump(lua_State* l);

	/// Make sure that the arguments match the argsCount number
	static void checkArgsCount(lua_State* l, I argsCount);

	/// Create a new LUA class
	static void createClass(lua_State* l, const char* className);

	/// Add new function in a class that it's already in the stack
	static void pushLuaCFuncMethod(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Add a new static function in the class.
	static void pushLuaCFuncStaticMethod(lua_State* l, const char* className, const char* name, lua_CFunction luafunc);

	/// Add a new function.
	static void pushLuaCFunc(lua_State* l, const char* name, lua_CFunction luafunc);

	/// Get a number from the stack.
	template<typename TNumber>
	static ANKI_USE_RESULT Error checkNumber(lua_State* l, I stackIdx, TNumber& number)
	{
		lua_Number lnum;
		Error err = checkNumberInternal(l, stackIdx, lnum);
		if(!err)
		{
			number = lnum;
		}
		return err;
	}

	/// Get a string from the stack.
	static ANKI_USE_RESULT Error checkString(lua_State* l, I32 stackIdx, const char*& out);

	/// Get some user data from the stack.
	/// The function uses the type signature to validate the type and not the
	/// typeName. That is supposed to be faster.
	static ANKI_USE_RESULT Error checkUserData(
		lua_State* l, I32 stackIdx, const char* typeName, I64 typeSignature, LuaUserData*& out);

	/// Allocate memory.
	static void* luaAlloc(lua_State* l, size_t size, U32 alignment);

	/// Free memory.
	static void luaFree(lua_State* l, void* ptr);

	template<typename TWrapedType>
	static I64 getWrappedTypeSignature();

	template<typename TWrapedType>
	static const char* getWrappedTypeName();

private:
	ScriptAllocator m_alloc;
	lua_State* m_l = nullptr;
	void* m_parent = nullptr; ///< Point to the ScriptManager

	static void* luaAllocCallback(void* userData, void* ptr, PtrSize osize, PtrSize nsize);

	static ANKI_USE_RESULT Error checkNumberInternal(lua_State* l, I32 stackIdx, lua_Number& number);
};
/// @}

} // end namespace anki
