// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCRIPT_LUA_BINDER_H
#define ANKI_SCRIPT_LUA_BINDER_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Allocator.h"
#include "anki/util/String.h"
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "Wrong LUA header included"
#endif
#include <functional>

namespace anki {

/// Lua binder class. A wrapper on top of LUA
class LuaBinder
{
public:
	template<typename T>
	using Allocator = HeapAllocator<T>;

	LuaBinder(Allocator<U8>& alloc, void* parent);
	~LuaBinder();

	/// @privatesection
	/// {
	lua_State* _getLuaState()
	{
		return m_l;
	}

	Allocator<U8> _getAllocator() const
	{
		return m_alloc;
	}

	void* _getParent() const
	{
		return m_parent;
	}

	/// Make sure that the arguments match the argsCount number
	static void checkArgsCount(lua_State* l, I argsCount);

	/// Create a new LUA class
	static void createClass(lua_State* l, const char* className);

	/// Add new function in a class that it's already in the stack
	static void pushLuaCFuncMethod(lua_State* l, const char* name,
		lua_CFunction luafunc);

	/// Add a new static function in the class
	static void pushLuaCFuncStaticMethod(lua_State* l, const char* className,
		const char* name, lua_CFunction luafunc);

	static void* luaAlloc(lua_State* l, size_t size);

	static void luaFree(lua_State* l, void* ptr);
	/// }

	/// Expose a variable to the lua state
	template<typename T>
	void exposeVariable(const char* name, T* y);

	/// Evaluate a file
	void evalFile(const CString& filename);

	/// Evaluate a string
	void evalString(const CString& str);

	/// For debugging purposes
	static void stackDump(lua_State* l);

private:
	Allocator<U8> m_alloc;
	lua_State* m_l = nullptr;
	void* m_parent = nullptr; ///< Point to the ScriptManager

	static void* luaAllocCallback(
		void* userData, void* ptr, PtrSize osize, PtrSize nsize);
};

//==============================================================================
/// lua userdata
class UserData
{
public:
	void* m_data = nullptr;
	Bool8 m_gc = false; ///< Garbage collection on?
};

} // end namespace anki

#endif
