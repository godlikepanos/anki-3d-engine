#!/usr/bin/python

# Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import os
import optparse
import xml.etree.ElementTree as et

# Globals
identation_level = 0
out_file = None

def parse_commandline():
	""" Parse the command line arguments """

	parser = optparse.OptionParser(usage = "usage: %prog [options]", description = "Create LUA bindings using XML")

	parser.add_option("-i", "--input", dest = "inp", type = "string", 
		help = "specify the XML files to parse. Seperate with :")

	(options, args) = parser.parse_args()

	if not options.inp:
		parser.error("argument is missing")

	return options.inp.split(":")

def type_sig(value):
	""" Calculate the signature of a type """
	return hash(value)

def get_base_fname(path):
	""" From path/to/a/file.ext return the "file" """
	return os.path.splitext(os.path.basename(path))[0]

def wglue(txt):
	""" Write glue code to the output """
	global out_file
	global identation_level
	out_file.write("%s%s\n" % ("\t" * identation_level, txt))

def ident(number):
	""" Increase or recrease identation for the wglue """
	global identation_level
	identation_level += number

def type_is_bool(type):
	""" Check if a type is boolean """

	return type == "Bool" or type == "Bool8" or type == "bool"

def type_is_number(type):
	""" Check if a type is number """

	numbers = ["U8", "U16", "U32", "U64", "I8", "I16", "I32", "I64", "U", "I", "PtrSize", "F32", "F64", \
		"int", "unsigned", "unsigned int", "short", "unsigned short", "uint", "float", "double"]

	it_is = False
	for num in numbers:
		if num == type:
			it_is = True
			break

	return it_is

def parse_type_decl(arg_txt):
	""" Parse an arg text """

	tokens = arg_txt.split(" ")
	tokens_size = len(tokens)

	type = tokens[tokens_size - 1]
	type_len = len(type)
	is_ptr = False
	is_ref = False
	if type[type_len - 1] == "&":
		is_ref = True
	elif type[type_len - 1] == "*":
		is_ptr = True

	if is_ref or is_ptr:
		type = type[:-1]

	is_const = False
	if tokens[0] == "const":
		is_const = True

	return (type, is_ref, is_ptr, is_const)

def ret(ret_el):
	""" Push return value """

	if ret_el is None:
		wglue("return 0;")
		return

	wglue("// Push return value")

	type_txt = ret_el.text
	(type, is_ref, is_ptr, is_const) = parse_type_decl(type_txt)

	if is_ptr:
		wglue("if(ANKI_UNLIKELY(ret == nullptr))")
		wglue("{")
		ident(1)
		wglue("lua_pushstring(l, \"Glue code returned nullptr\");")
		wglue("return -1;")
		ident(-1)
		wglue("}")
		wglue("")

	if type_is_bool(type):
		wglue("lua_pushboolean(l, ret);")
	elif type_is_number(type):
		wglue("lua_pushnumber(l, ret);")
	elif type == "char" or type == "CString":
		wglue("lua_pushstring(l, &ret[0]);")
	elif type == "Error":
		wglue("if(ANKI_UNLIKELY(ret))")
		wglue("{")
		ident(1)
		wglue("lua_pushstring(l, \"Glue code returned an error\");")
		wglue("return -1;")
		ident(-1)
		wglue("}")
		wglue("")
		wglue("lua_pushnumber(l, ret);")
	else:
		if is_ptr or is_ref:
		 	wglue("voidp = lua_newuserdata(l, sizeof(UserData));")
			wglue("ud = static_cast<UserData*>(voidp);")
			wglue("luaL_setmetatable(l, \"%s\");" % type)

			if is_ptr:
				wglue("ud->initPointed(%d, const_cast<%s*>(ret));" % (type_sig(type), type))
			elif is_ref:
				wglue("ud->initPointed(%d, const_cast<%s*>(&ret));" % (type_sig(type), type))
		else:
			wglue("size = UserData::computeSizeForGarbageCollected<%s>();" % type)
			wglue("voidp = lua_newuserdata(l, size);")
			wglue("luaL_setmetatable(l, \"%s\");" % type)

			wglue("ud = static_cast<UserData*>(voidp);")
			wglue("ud->initGarbageCollected(%d);" % type_sig(type))

			wglue("::new(ud->getData<%s>()) %s(std::move(ret));" % (type, type))

	wglue("")
	wglue("return 1;")

def arg(arg_txt, stack_index, index):
	""" Write the pop code for a single argument """

	(type, is_ref, is_ptr, is_const) = parse_type_decl(arg_txt)

	if type_is_bool(type) or type_is_number(type):
		wglue("%s arg%d;" % (type, index))
		wglue("if(LuaBinder::checkNumber(l, %d, arg%d))" % (stack_index, index))
		wglue("{")
		ident(1)
		wglue("return -1;")
		ident(-1)
		wglue("}")
	elif type == "char" or type == "CString":
		wglue("const char* arg%d;" % index)
		wglue("if(LuaBinder::checkString(l, %d, arg%d))" % (stack_index, index))
		wglue("{")
		ident(1)
		wglue("return -1;")
		ident(-1)
		wglue("}")
	else:
		wglue("if(LuaBinder::checkUserData(l, %d, \"%s\", %d, ud))" % (stack_index, type, type_sig(type)))
		wglue("{")
		ident(1)
		wglue("return -1;")
		ident(-1)
		wglue("}")
		wglue("")

		wglue("%s* iarg%d = ud->getData<%s>();" % (type, index, type))

		if is_ptr:
			wglue("%s arg%d(iarg%d);" % (arg_txt, index, index))
		else:
			wglue("%s arg%d(*iarg%d);" % (arg_txt, index, index))


def args(args_el, stack_index):
	""" Write the pop code for argument parsing and return the arg list """

	if args_el is None:
		return ""

	wglue("// Pop arguments")
	arg_index = 0

	# Do the work
	args_str = ""
	arg_index = 0
	for arg_el in args_el.iter("arg"):
		arg(arg_el.text, stack_index, arg_index)
		args_str += "arg%d, " % arg_index
		wglue("")
		stack_index += 1
		arg_index += 1

	if len(args_str) > 0:
		args_str = args_str[:-2]

	return args_str

def check_args(args_el, bias):
	""" Check number of args. Call that first because it throws error """

	if args_el is None:
		wglue("LuaBinder::checkArgsCount(l, %d);" % bias)
	else:
		count = 0
		for arg_el in args_el.iter("arg"):
			count += 1

		wglue("LuaBinder::checkArgsCount(l, %d);" % (bias + count))

	wglue("")

def get_meth_alias(meth_el):
	""" Return the method alias """

	meth_name = meth_el.get("name")

	if meth_name == "operator+":
		meth_alias = "__add"
	elif meth_name == "operator-":
		meth_alias = "__sub"
	elif meth_name == "operator*":
		meth_alias = "__mul"
	elif meth_name == "operator/":
		meth_alias = "__div"
	elif meth_name == "operator==":
		meth_alias = "__eq"
	elif meth_name == "operator<":
		meth_alias = "__lt"
	elif meth_name == "operator<=":
		meth_alias = "__le"
	elif meth_name == "operator>":
		meth_alias = "__gt"
	elif meth_name == "operator>=":
		meth_alias = "__ge"
	elif meth_name == "operator=":
		meth_alias = "copy"
	else:
		meth_alias = meth_name

	meth_alias_txt = meth_el.get("alias")
	if meth_alias_txt is not None:
		meth_alias = meth_alias_txt

	return meth_alias

def write_local_vars():
	wglue("UserData* ud;")
	wglue("(void)ud;")
	wglue("void* voidp;")
	wglue("(void)voidp;")
	wglue("PtrSize size;")
	wglue("(void)size;")
	wglue("")

def method(class_name, meth_el):
	""" Handle a method """

	meth_name = meth_el.get("name")
	meth_alias = get_meth_alias(meth_el)

	wglue("/// Pre-wrap method %s::%s." % (class_name, meth_name))
	wglue("static inline int pwrap%s%s(lua_State* l)" % (class_name, meth_alias))
	wglue("{")
	ident(1)
	write_local_vars()

	check_args(meth_el.find("args"), 1)

	# Get this pointer
	wglue("// Get \"this\" as \"self\"")
	wglue("if(LuaBinder::checkUserData(l, 1, classname%s, %d, ud))" % (class_name, type_sig(class_name)))
	wglue("{")
	ident(1)
	wglue("return -1;")
	ident(-1)
	wglue("}")
	wglue("")
	wglue("%s* self = ud->getData<%s>();" % (class_name, class_name))
	wglue("")

	args_str = args(meth_el.find("args"), 2)

	# Return value
	ret_txt = None
	ret_el = meth_el.find("return")
	if ret_el is not None:
		ret_txt = ret_el.text

	# Method call
	wglue("// Call the method")
	call = meth_el.find("overrideCall")
	if call is not None:
		call = call.text

	if call is not None:
		wglue("%s" % call)
	else:
		if ret_txt is None:
			wglue("self->%s(%s);" % (meth_name, args_str))
		else:
			wglue("%s ret = self->%s(%s);" % (ret_txt, meth_name, args_str))

	wglue("")
	ret(ret_el)

	ident(-1)
	wglue("}")
	wglue("")

	# Write the actual function
	wglue("/// Wrap method %s::%s." % (class_name, meth_name))
	wglue("static int wrap%s%s(lua_State* l)" % (class_name, meth_alias))
	wglue("{")
	ident(1)
	wglue("int res = pwrap%s%s(l);" % (class_name, meth_alias))
	wglue("if(res >= 0)")
	wglue("{")
	ident(1)
	wglue("return res;")
	ident(-1)
	wglue("}")
	wglue("")
	wglue("lua_error(l);")
	wglue("return 0;")
	ident(-1)
	wglue("}")
	wglue("")

def static_method(class_name, meth_el):
	""" Handle a static method """

	meth_name = meth_el.get("name")
	meth_alias = get_meth_alias(meth_el)

	wglue("/// Pre-wrap static method %s::%s." % (class_name, meth_name))
	wglue("static inline int pwrap%s%s(lua_State* l)" % (class_name, meth_alias))
	wglue("{")
	ident(1)
	write_local_vars()

	check_args(meth_el.find("args"), 0)

	# Args
	args_str = args(meth_el.find("args"), 1)

	# Return value
	ret_txt = None
	ret_el = meth_el.find("return")
	if ret_el is not None:
		ret_txt = ret_el.text

	# Method call
	wglue("// Call the method")
	if ret_txt is None:
		wglue("%s::%s(%s);" % (class_name, meth_name, args_str))
	else:
		wglue("%s ret = %s::%s(%s);" % (ret_txt, class_name, meth_name, args_str))

	wglue("")
	ret(ret_el)

	ident(-1)
	wglue("}")
	wglue("")

	# Write the actual function
	wglue("/// Wrap static method %s::%s." % (class_name, meth_name))
	wglue("static int wrap%s%s(lua_State* l)" % (class_name, meth_alias))
	wglue("{")
	ident(1)
	wglue("int res = pwrap%s%s(l);" % (class_name, meth_alias))
	wglue("if(res >= 0)")
	wglue("{")
	ident(1)
	wglue("return res;")
	ident(-1)
	wglue("}")
	wglue("")
	wglue("lua_error(l);")
	wglue("return 0;")
	ident(-1)
	wglue("}")
	wglue("")

def constructor(constr_el, class_name):
	""" Handle constructor """

	wglue("/// Pre-wrap constructor for %s." % (class_name))
	wglue("static inline int pwrap%sCtor(lua_State* l)" % class_name)
	wglue("{")
	ident(1)
	write_local_vars()

	check_args(constr_el.find("args"), 0)

	# Args
	args_str = args(constr_el.find("args"), 1)

	# Create new userdata
	wglue("// Create user data")

	wglue("size = UserData::computeSizeForGarbageCollected<%s>();" % class_name)
	wglue("voidp = lua_newuserdata(l, size);")
	wglue("luaL_setmetatable(l, classname%s);" % class_name)
	wglue("ud = static_cast<UserData*>(voidp);")
	wglue("ud->initGarbageCollected(%d);" % type_sig(class_name))
	wglue("::new(ud->getData<%s>()) %s(%s);" % (class_name, class_name, args_str))
	wglue("")

	wglue("return 1;")

	ident(-1)
	wglue("}")
	wglue("")

	# Write the actual function
	wglue("/// Wrap constructor for %s." % class_name)
	wglue("static int wrap%sCtor(lua_State* l)" % class_name)
	wglue("{")
	ident(1)
	wglue("int res = pwrap%sCtor(l);" % class_name)
	wglue("if(res >= 0)")
	wglue("{")
	ident(1)
	wglue("return res;")
	ident(-1)
	wglue("}")
	wglue("")
	wglue("lua_error(l);")
	wglue("return 0;")
	ident(-1)
	wglue("}")
	wglue("")

def destructor(class_name):
	""" Create a destructor """

	wglue("/// Wrap destructor for %s." % (class_name))
	wglue("static int wrap%sDtor(lua_State* l)" % class_name)
	wglue("{")
	ident(1)
	write_local_vars();

	wglue("LuaBinder::checkArgsCount(l, 1);")
	wglue("if(LuaBinder::checkUserData(l, 1, classname%s, %d, ud))" % (class_name, type_sig(class_name)))
	wglue("{")
	ident(1)
	wglue("return -1;")
	ident(-1)
	wglue("}")
	wglue("");

	wglue("if(ud->isGarbageCollected())")
	wglue("{")
	ident(1)
	wglue("%s* inst = ud->getData<%s>();" % (class_name, class_name))
	wglue("inst->~%s();" % class_name)
	ident(-1)
	wglue("}")
	wglue("")

	wglue("return 0;")

	ident(-1)
	wglue("}")
	wglue("")

def class_(class_el):
	""" Create a class """

	class_name = class_el.get("name")

	# Write class decoration and stuff
	wglue("static const char* classname%s = \"%s\";" % (class_name, class_name))
	wglue("")
	wglue("template<>")
	wglue("I64 LuaBinder::getWrappedTypeSignature<%s>()" % class_name)
	wglue("{")
	ident(1)
	wglue("return %d;" % type_sig(class_name))
	ident(-1)
	wglue("}")
	wglue("")
	wglue("template<>")
	wglue("const char* LuaBinder::getWrappedTypeName<%s>()" % class_name)
	wglue("{")
	ident(1)
	wglue("return classname%s;" % class_name)
	ident(-1)
	wglue("}")
	wglue("")

	# Constructor declarations
	has_constructor = False
	constructor_el = class_el.find("constructor")
	if constructor_el is not None:
		constructor(constructor_el, class_name)
		has_constructor = True

	# Destructor declarations
	if has_constructor:
		destructor(class_name)

	# Methods LUA C functions declarations
	meth_names_aliases = []
	meths_el = class_el.find("methods")
	if meths_el is not None:
		for meth_el in meths_el.iter("method"):
			is_static = meth_el.get("static")
			is_static = is_static is not None and is_static == "1"

			if is_static:
				static_method(class_name, meth_el)
			else:
				method(class_name, meth_el)

			meth_name = meth_el.get("name")
			meth_alias = get_meth_alias(meth_el)
			meth_names_aliases.append([meth_name, meth_alias, is_static])

	# Start class declaration
	wglue("/// Wrap class %s." % class_name)
	wglue("static inline void wrap%s(lua_State* l)" % class_name)
	wglue("{")
	ident(1)
	wglue("LuaBinder::createClass(l, classname%s);" % class_name)

	# Register constructor
	if has_constructor:
		wglue("LuaBinder::pushLuaCFuncStaticMethod(l, classname%s, \"new\", wrap%sCtor);" % (class_name, class_name))

	# Register destructor
	if has_constructor:
		wglue("LuaBinder::pushLuaCFuncMethod(l, \"__gc\", wrap%sDtor);" % class_name)

	# Register methods
	if len(meth_names_aliases) > 0:
		for meth_name_alias in meth_names_aliases:
			meth_alias = meth_name_alias[1]
			is_static = meth_name_alias[2]
			if is_static:
				wglue("LuaBinder::pushLuaCFuncStaticMethod(l, classname%s, \"%s\", wrap%s%s);" \
					% (class_name, meth_alias, class_name, meth_alias))
			else:
				wglue("LuaBinder::pushLuaCFuncMethod(l, \"%s\", wrap%s%s);" % (meth_alias, class_name, meth_alias))

	wglue("lua_settop(l, 0);")

	ident(-1)
	wglue("}")
	wglue("")

def function(func_el):
	""" Handle a plain function """

	func_name = func_el.get("name")
	func_alias = get_meth_alias(func_el)

	wglue("/// Pre-wrap function %s." % func_name)
	wglue("static inline int pwrap%s(lua_State* l)" % func_alias)
	wglue("{")
	ident(1)
	write_local_vars()

	check_args(func_el.find("args"), 0)

	# Args
	args_str = args(func_el.find("args"), 1)

	# Return value
	ret_txt = None
	ret_el = func_el.find("return")
	if ret_el is not None:
		ret_txt = ret_el.text

	# Call
	wglue("// Call the function")
	call = func_el.find("overrideCall")
	if call is not None:
		call = call.text

	if call is not None:
		wglue("%s" % call)
	else:
		if ret_txt is None:
			wglue("%s(%s);" % (func_name, args_str))
		else:
			wglue("%s ret = %s(%s);" % (ret_txt, func_name, args_str))

	wglue("")
	ret(ret_el)

	ident(-1)
	wglue("}")
	wglue("")

	# Write the actual function
	wglue("/// Wrap function %s." % func_name)
	wglue("static int wrap%s(lua_State* l)" % func_alias)
	wglue("{")
	ident(1)
	wglue("int res = pwrap%s(l);" % func_alias)
	wglue("if(res >= 0)")
	wglue("{")
	ident(1)
	wglue("return res;")
	ident(-1)
	wglue("}")
	wglue("")
	wglue("lua_error(l);")
	wglue("return 0;")
	ident(-1)
	wglue("}")
	wglue("")

def main():
	""" Main function """

	global out_file
	filenames = parse_commandline()

	for filename in filenames:
		out_filename = get_base_fname(filename) + ".cpp"
		out_file = open(out_filename, "w")

		tree = et.parse(filename)
		root = tree.getroot()

		# Head
		head = root.find("head")
		if head is not None:
			wglue("%s" % head.text)
			wglue("")

		# Classes
		class_names = []
		for cls in root.iter("classes"):
			for cl in cls.iter("class"):
				class_(cl)
				class_names.append(cl.get("name"))

		# Functions
		func_names = []
		for fs in root.iter("functions"):
			for f in fs.iter("function"):
				function(f)
				func_names.append(f.get("name"))

		# Wrap function
		wglue("/// Wrap the module.")
		wglue("void wrapModule%s(lua_State* l)" % get_base_fname(filename))
		wglue("{")
		ident(1)
		for class_name in class_names:
			wglue("wrap%s(l);" % class_name)
		for func_name in func_names:
			wglue("LuaBinder::pushLuaCFunc(l, \"%s\", wrap%s);" % (func_name, func_name))
		ident(-1)
		wglue("}")
		wglue("")

		# Tail
		tail = root.find("tail")
		if tail is not None:
			wglue("%s" % tail.text)
			wglue("")

		out_file.close()

if __name__ == "__main__":
	main()

