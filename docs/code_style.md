This document describes the code style of AnKi 3D Engine.


Comments
========

All comments are C++ like:

	// Some comment

And for doxygen comments:

	/// @brief blah blah
	/// @param blah blah

Naming Conventions
==================

**Variables, functions and methods** are lower camelCase.

	functionDoSomething(someVariableA, someVariableB);

All **types** are PascalCase and that includes classes, structs, typedefs and enums.

	enum MyEnum {...};
	class MyClass {...};
	using MyType = int;

All **constant expressions** are SCREAMING_SNAKE_CASE and that includes enumerants.

	constexpr int MY_CONSTANT = ...;

	enum MyEnum
	{
		ENUMERANT_A
	};

All **macros and defines** should be SCREAMING_SNAKE_CASE and they should have an `ANKI_` prefix.

	ANKI_ASSERT(...);

	#define ANKI_MAX_SOMETHING ...

All **template arguments** should start with `T`.

	template<typename TSomething, typename TOther, U32 T_SOME_CONST>

The **source files** are always PascalCase.

	ThreadHive.h
	MyShaderProgram.ankiprog

All **function and method names** should form a sentence with at least one verb.

	doSomething(...);
	getSomething();
	calculateSomething(...);

**No hungarian notations** with 2 exceptions.

- All member variables have the `m_` prefix.
- All global variables have the `g_` prefix.

In GLSL there are more exceptions:

- All uniforms (buffers, textures, images, push constants) and storage buffers have the `u_` prefix.
- All input globals the `in_` prefix.
- All output globals the `out_` prefix.
- All shared storage the `s_` prefix.
- All blocks (storage or uniform) block names the  `b_` prefix.

**Variables that act as a measure for quantity** should have the `count` suffix. Not `num` or `numberOf` or similar.

	int appleCount = ...; // YES
	int appleNum = ...;   // NO
	int numApples = ...;  // NO

C++ rules
=========

**Always use strongly typed enums**. If you need to relax the rules use the `ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS` macro.

	enum class MyEnum : uint {...};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MyEnum);

**Never use `typedef`** to define types. Use `using` instead.

	using U32 = uint32_t;
	using Func = void(*)(int, int);

**Never use C-style casting**. Use static_cast, reinterpret_cast or const_cast for most cases and constructor-like cast for fudmental types.

	Derived* d = static_cast<Derived*>(base);

	uint i = uint(1.1f); // uint is fudmental type

**Always use `constexpr`** when applicable. Never use defines for constants.

	constexpr uint SOME_CONST = ...;

**Never use `struct`** and always use `class` instead. Since it's difficult to come up with rules on when to use struct over class always use class and be done with it.

	class MyTrivialType
	{
	public:
		int m_x;
		int m_y;
	};

**Avoid using `auto`**. It's only allowed in some templates and in iterators. auto makes reading code difficult.

**Includes should always have the full file path**. This avoids issues with files of similar name. Non-AnKi includes follow AnKi includes.

	#include <anki/util/Thread.h>
	#include <anki/util/WeakArray.h>
	#include <anki/util/Allocator.h>
	#include <cstdio>

**Never use public nested classes**. Only private nested classes are allowed. Nested classes cannot be forward declared.

	class MyClass
	{
	public:
		// NO!!!!!!!!!!!
		class Nested {...};

	private:
		// It's fine
		class Nested {...};

		// Also fine
		class
		{
			...
		} m_myUnamedThing;
	};

Try to **use `explicit`** in constructors for extra safety. Don't use explicit on copy constructors.

AnKi uses **const correctness** throughout and that's mandatory. Always use `const`. The only place where you don't have to is for function and method parameters.

	uint doSomething(uint noNeedForConst)
	{
		const uint bla = noNeedForConst + 10;
		return blah;
	}

**Access types in a class have a specified order**. First `public` then `protected` and last `private`. There are some exceptions where this rule has to break.

	class MyClass
	{
	public:
		...
	protected:
		...
	private:
		...
	};

Always **use AnKi's fudmental types** (U32, I32, F32, Bool etc etc). For integers and if you don't care about the size of the integer you can use the type `U` for unsinged or `I` for signed. Both of them are at least 32bit.

	for(U32 i = 0; i < 10; ++i) {...}

If you have to overload the operator Bool always use **`explicit operator Bool` operator** overloading.

	explicit operator Bool() const {...}

The use of **references and pointers** is govern by some rules. Always use references except when you transfer or share ownership where you should use pointers. Pointers are also allowed for optional data that can be nullptr.

	// ptr is a pointer so you CAN NOT destroy it after a call to doSomething is done
	// ref is a reference so you CAN destroy it after a call to doSomething is done
	void Foo::doSomething(TypeA* ptr, TypeB& ref)
	{
		// Store the ptr somewhere
		m_someMember = ptr;

		// Won't store the ref
		ref += ...;
	}

Always **use `nullptr`**.

	void* ptr = nullptr;

**Never use C style arrays**. Use the `Array<>` template instead.

	Array<MyType, 10> myArray;

**Don't use STL**. AnKi has a replacement for most STL templates. There are some exceptions though.

- Can use `type_traights`.
- Some from `utility` eg std::move.
- Some from `algorithm` eg std::sort.

Always try to **comment parts of the source code**. Self documenting code is never enough.

Always use `override` or `final` on virtual methods and try to use `final` on classes when applicable.

	class Foo final
	{
	public:
		void myVirtual(...) override {...}
	};

Code formatting
===============

clang-format deals with code formatting. To format the whole source tree type the following in a terminal:

	$ cd /path/to/anki
	$ ./tools/format_source.sh

If you need to format on Windows do the same from a WSL terminal.

Some cases that are not handled by clang-format follow.

**Always use curly braces** on single line expressions.

	// NO!!!!!!!!!!!
	if(something) doSomething();

	// YES!!!!!!!!!!!!!!
	if(something)
	{
		doSomething();
	}

Always place the **condition of a conditional ternary operator** inside parenthesis.

	// NO!!!!!!!!!!!
	a = b ? 1 : 0;

	// YES!!!!!!!!!!!!!!
	a = (b) ? 1 : 0;
