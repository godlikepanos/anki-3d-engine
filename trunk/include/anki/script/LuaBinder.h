#ifndef ANKI_SCRIPT_LUA_BINDER_H
#define ANKI_SCRIPT_LUA_BINDER_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include <lua.hpp>
#ifndef ANKI_LUA_HPP
#	error "See file"
#endif

namespace anki {

/// Internal lua stuff
namespace lua_detail {

//==============================================================================
/// lua userdata
struct UserData
{
	void* ptr = nullptr;
	Bool8 gc = false; ///< Garbage collection on?
};

//==============================================================================
// Flags
constexpr U32 LF_NONE = 0;
constexpr U32 LF_TRANFER_OWNERSHIP = 1;

//==============================================================================
/// Class identification
template<typename Class>
struct ClassProxy
{
	static const char* NAME; ///< Used to check the signature of the user data

	static const char* getName()
	{
		ANKI_ASSERT(NAME != nullptr && "Class already wrapped elsewhere");
		return NAME;
	}
};

template<typename Class>
const char* ClassProxy<Class>::NAME = nullptr;

//==============================================================================

/// Make sure that the arguments match the argsCount number
extern void checkArgsCount(lua_State* l, I argsCount);

/// Create a new LUA class
extern void createClass(lua_State* l, const char* className);

/// Add new function in a class that it's already in the stack
extern void pushCFunctionMethod(lua_State* l, const char* name,
	lua_CFunction luafunc);

/// Add a new static function in the class
extern void pushCFunctionStatic(lua_State* l, const char* className,
	const char* name, lua_CFunction luafunc);

//==============================================================================
/// Used mainly to push a method's return value to the stack
template<typename Class, U32 flags>
struct PushStack
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = new Class(x);
		d->gc = true;
	}
};

// Specialization ref
template<typename Class, U32 flags>
struct PushStack<Class&, flags>
{
	void operator()(lua_State* l, Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = &x;
		d->gc = flags & LF_TRANFER_OWNERSHIP;
	}
};

// Specialization const ref
template<typename Class, U32 flags>
struct PushStack<const Class&, flags>
{
	void operator()(lua_State* l, const Class& x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = &const_cast<Class&>(x);
		d->gc = flags & LF_TRANFER_OWNERSHIP;
	}
};

// Specialization ptr
template<typename Class, U32 flags>
struct PushStack<Class*, flags>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = x;
		d->gc = flags & LF_TRANFER_OWNERSHIP;
	}
};

// Specialization const ptr
template<typename Class, U32 flags>
struct PushStack<const Class*, flags>
{
	void operator()(lua_State* l, Class* x)
	{
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		luaL_setmetatable(l, ClassProxy<Class>::getName());
		d->ptr = const_cast<Class*>(x);
		d->gc = flags & LF_TRANFER_OWNERSHIP;
	}
};

// Specialization const char*
template<U32 flags>
struct PushStack<const char*, flags>
{
	void operator()(lua_State* l, const char* x)
	{
		lua_pushstring(l, x);
	}
};

#define ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<U32 flags> \
	struct PushStack<Type_, flags> \
	{ \
		void operator()(lua_State* l, Type_ x) \
		{ \
			luafunc_(l, x); \
		} \
	};

ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I8, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I16, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(I64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U8, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U16, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(U64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(F64, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(F32, lua_pushnumber)
ANKI_PUSH_STACK_TEMPLATE_SPECIALIZATION(Bool, lua_pushnumber)

//==============================================================================
/// Used to get the function arguments from the stack
template<typename Class, I stackIndex>
struct StackGet
{
	Class operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return Class(*a);
	}
};

// Specialization const ref
template<typename Class, I stackIndex>
struct StackGet<const Class&, stackIndex>
{
	const Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return *a;
	}
};

// Specialization ref
template<typename Class, I stackIndex>
struct StackGet<Class&, stackIndex>
{
	Class& operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->ptr);
		return *a;
	}
};

// Specialization const ptr
template<typename Class, I stackIndex>
struct StackGet<const Class*, stackIndex>
{
	const Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		const Class* a = reinterpret_cast<const Class*>(udata->ptr);
		return a;
	}
};

// Specialization ptr
template<typename Class, I stackIndex>
struct StackGet<Class*, stackIndex>
{
	Class* operator()(lua_State* l)
	{
		UserData* udata = (UserData*)luaL_checkudata(l, stackIndex, 
			ClassProxy<Class>::getName());

		Class* a = reinterpret_cast<Class*>(udata->ptr);
		return a;
	}
};

#define ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(Type_, luafunc_) \
	template<I stackIndex> \
	struct StackGet<Type_, stackIndex> \
	{ \
		Type_ operator()(lua_State* l) \
		{ \
			return luafunc_(l, stackIndex); \
		} \
	};

ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(F32, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(F64, luaL_checknumber)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I8, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I16, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I32, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(I64, luaL_checkinteger)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U8, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U16, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U32, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(U64, luaL_checkunsigned)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(const char*, luaL_checkstring)
ANKI_STACK_GET_TEMPLATE_SPECIALIZATION(Bool, luaL_checkunsigned)

//==============================================================================
/// Call a function
template<typename T, U32 flags>
struct CallFunction;

// R (_1)
template<typename TReturn, typename Arg0, U32 flags>
struct CallFunction<TReturn (*)(Arg0), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2)
template<typename TReturn, typename Arg0, typename Arg1, U32 flags>
struct CallFunction<TReturn (*)(Arg0, Arg1), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0, Arg1))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l), 
			StackGet<Arg1, 2>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// R (_1, _2, _3)
template<typename TReturn, typename Arg0, typename Arg1, typename Arg2,
	U32 flags>
struct CallFunction<TReturn (*)(Arg0, Arg1, Arg2), flags>
{
	int operator()(lua_State* l, TReturn (*func)(Arg0, Arg1, Arg2))
	{
		TReturn out = (*func)(StackGet<Arg0, 1>()(l),
			StackGet<Arg1, 2>()(l), StackGet<Arg2, 3>()(l));
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// void (_1)
template<typename Arg0, U32 flags>
struct CallFunction<void (*)(Arg0), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0))
	{
		(*func)(StackGet<Arg0, 1>()(l));
		return 0;
	}
};

// void (_1, _2)
template<typename Arg0, typename Arg1, U32 flags>
struct CallFunction<void (*)(Arg0, Arg1), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l));
		return 0;
	}
};

// void (_1, _2, _3)
template<typename Arg0, typename Arg1, typename Arg2, U32 flags>
struct CallFunction<void (*)(Arg0, Arg1, Arg2), flags>
{
	int operator()(lua_State* l, void (*func)(Arg0, Arg1, Arg2))
	{
		(*func)(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l),
			StackGet<Arg2, 3>()(l));
		return 0;
	}
};

// R (void)
template<typename TReturn, U32 flags>
struct CallFunction<TReturn (*)(void), flags>
{
	int operator()(lua_State* l, TReturn (*func)(void))
	{
		TReturn out = (*func)();
		PushStack<TReturn, flags> ps;
		ps(l, out);
		return 1;
	}
};

// void (void)
template<U32 flags>
struct CallFunction<void (*)(void), flags>
{
	int operator()(lua_State* /*l*/, void (*func)(void))
	{
		(*func)();
		return 0;
	}
};

//==============================================================================
/// Call constructor
template<typename Class, typename... Args>
struct CallConstructor;

// none
template<typename Class>
struct CallConstructor<Class>
{
	Class* operator()(lua_State*)
	{
		return new Class();
	}
};

// _1
template<typename Class, typename Arg0>
struct CallConstructor<Class, Arg0>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l));
	}
};

// _1, _2
template<typename Class, typename Arg0, typename Arg1>
struct CallConstructor<Class, Arg0, Arg1>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l));
	}
};

// _1, _2, _3
template<typename Class, typename Arg0, typename Arg1, typename Arg2>
struct CallConstructor<Class, Arg0, Arg1, Arg2>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l),
			StackGet<Arg2, 3>()(l));
	}
};

// _1, _2, _3, _4
template<typename Class, typename Arg0, typename Arg1, typename Arg2,
	typename Arg3>
struct CallConstructor<Class, Arg0, Arg1, Arg2, Arg3>
{
	Class* operator()(lua_State* l)
	{
		return new Class(StackGet<Arg0, 1>()(l), StackGet<Arg1, 2>()(l),
			StackGet<Arg2, 3>()(l), StackGet<Arg3, 4>()(l));
	}
};

//==============================================================================
/// Make a method function. Used like this:
/// @code
/// Foo foo; // An instance
/// MethodFunctionalizer<decltype(Foo::bar)>::In<Foo::bar>::func(&foo, 123);
/// // Equivelent of:
/// foo.bar(123);
/// @endcode
template<typename T>
struct MethodFunctionalizer;

template<typename Class, typename TReturn, typename... Args>
struct MethodFunctionalizer<TReturn (Class::*)(Args...)>
{
	template<TReturn (Class::* method)(Args...)>
	struct In
	{
		static TReturn func(Class* self, Args... args)
		{
			return (self->*method)(args...);
		}
	};
};

template<typename Class, typename TReturn, typename... Args>
struct MethodFunctionalizer<TReturn (Class::*)(Args...) const>
{
	template<TReturn (Class::* method)(Args...) const>
	struct In
	{
		static TReturn func(const Class* self, Args... args)
		{
			return (self->*method)(args...);
		}
	};
};

//==============================================================================
/// Signature for constructor
template<typename Class, typename... Args>
struct ConstructorSignature
{
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args));
		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));

		luaL_setmetatable(l, ClassProxy<Class>::getName());

		d->ptr = CallConstructor<Class, Args...>()(l);
		d->gc = true;
		return 1;
	}
};

//==============================================================================
/// Destructor signature
template<typename Class>
struct DestructorSignature
{
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, 1);
		UserData* d = (UserData*)luaL_checkudata(l, 1, 
			ClassProxy<Class>::getName());
		if(d->gc)
		{
			Class* obj = reinterpret_cast<Class*>(d->ptr);
			delete obj;
		}
		return 0;
	}
};

//==============================================================================
/// Function signature
template<U32 flags, typename T>
struct FunctionSignature;

template<U32 flags, typename TReturn, typename... Args>
struct FunctionSignature<flags, TReturn (*)(Args...)>
{
	template<TReturn (* func)(Args...)>
	static int luafunc(lua_State* l)
	{
		checkArgsCount(l, sizeof...(Args));
		auto ff = func; // A hack that saves GCC
		CallFunction<decltype(ff), flags> cf;
		return cf(l, func);
	}
};

} // end namespace lua_detail

//==============================================================================
// Macros

/// Don't use it directly
#define ANKI_LUA_DESTRUCTOR() \
	lua_detail::pushCFunctionMethod(l_, "__gc", \
		&lua_detail::DestructorSignature<Class>::luafunc);

/// Start wrapping a class. Don't add a destructor (if for example the class 
/// has a private derstructor)
#define ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(luaBinder_, Class_) { \
	typedef Class_ Class; \
	lua_State* l_ = luaBinder_._getLuaState(); \
	lua_detail::ClassProxy<Class>::NAME = #Class_; \
	lua_detail::createClass(l_, lua_detail::ClassProxy<Class>::getName());

/// Start wrapping a class
#define ANKI_LUA_CLASS_BEGIN(luaBinder_, Class_) \
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(luaBinder_, Class_) \
	ANKI_LUA_DESTRUCTOR()

/// End wrapping a class
#define ANKI_LUA_CLASS_END() lua_settop(l_, 0); }

/// Define a constructor. Call it from lua @code a = Foo.new(...) @endcode.
#define ANKI_LUA_CONSTRUCTOR(...) \
	lua_detail::pushCFunctionStatic(l_, \
		lua_detail::ClassProxy<Class>::getName(), "new", \
		&lua_detail::ConstructorSignature<Class, __VA_ARGS__>::luafunc);

/// Define a static method with flags
#define ANKI_LUA_STATIC_METHOD_FLAGS(name_, smethodPtr_, flags_) \
	lua_detail::pushCFunctionStatic(l_, \
		lua_detail::ClassProxy<Class>::getName(), name_, \
		&lua_detail::FunctionSignature<flags_, \
		decltype(smethodPtr_)>::luafunc<smethodPtr_>);

/// Define a static method no flags
#define ANKI_LUA_STATIC_METHOD(name_, smethodPtr_) \
	ANKI_LUA_STATIC_METHOD_FLAGS(name_, smethodPtr_, lua_detail::LF_NONE)

/// Define a function as method with flags
#define ANKI_LUA_FUNCTION_AS_METHOD_FLAGS(name_, funcPtr_, flags_) \
	lua_detail::pushCFunctionMethod(l_, name_, \
		&lua_detail::FunctionSignature<flags_, \
		decltype(funcPtr_)>::luafunc<funcPtr_>);

/// Define a function as method no flags
#define ANKI_LUA_FUNCTION_AS_METHOD(name_, funcPtr_) \
	ANKI_LUA_FUNCTION_AS_METHOD_FLAGS(name_, funcPtr_, lua_detail::LF_NONE)

/// Define a method with flags
#define ANKI_LUA_METHOD_FLAGS(name_, methodPtr_, flags_) \
	ANKI_LUA_FUNCTION_AS_METHOD_FLAGS(name_, \
		&lua_detail::MethodFunctionalizer< \
		decltype(methodPtr_)>::In<methodPtr_>::func, flags_)

/// Define a method no flags
#define ANKI_LUA_METHOD(name_, methodPtr_) \
	ANKI_LUA_METHOD_FLAGS(name_, methodPtr_, lua_detail::LF_NONE)

//==============================================================================
/// Lua binder class. A wrapper on top of LUA
class LuaBinder
{
public:
	LuaBinder();
	~LuaBinder();

	/// @name Accessors
	/// {
	lua_State* _getLuaState()
	{
		return l;
	}
	/// }

	/// Expose a variable to the lua state
	template<typename T>
	void exposeVariable(const char* name, T* y)
	{
		using namespace lua_detail;

		UserData* d = (UserData*)lua_newuserdata(l, sizeof(UserData));
		d->ptr = y;
		d->gc = false;
		luaL_setmetatable(l, ClassProxy<T>::getName());
		lua_setglobal(l, name);
	}

	/// Evaluate a file
	void evalFile(const char* filename);
	/// Evaluate a string
	void evalString(const char* str);

	/// For debugging purposes
	static void stackDump(lua_State* l);

private:
	lua_State* l = nullptr;
};

} // end namespace anki

#endif
