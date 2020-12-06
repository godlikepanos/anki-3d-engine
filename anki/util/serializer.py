#!/usr/bin/python3

# Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import optparse
import xml.etree.ElementTree as et
import os
import copy


class Context:
    __slots__ = ["input_filenames", "output_filename", "output_file", "identation_level"]

    def __init__(self):
        self.input_filenames = None
        self.output_filename = None
        self.output_file = None
        self.identation_level = 0


ctx = Context()


class MemberInfo:
    __slots__ = ["name", "base_type", "array_size", "comment", "pointer", "constructor"]

    def __init__(self):
        self.name = None
        self.base_type = None
        self.array_size = "1"
        self.comment = None
        self.pointer = False
        self.constructor = None

    def is_dynamic_array(self, member_arr):
        if not self.pointer:
            return False

        for member in member_arr:
            if member.name == self.array_size:
                return True

        return False

    def is_pointer(self, member_arr):
        return self.pointer and not self.is_dynamic_array(member_arr)


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]", description="Create serialization code using XML")

    parser.add_option("-i",
                      "--input",
                      dest="inp",
                      type="string",
                      help="specify the XML files to parse. Seperate with :")

    parser.add_option("-o", "--output", dest="out", type="string", help="specify the .h file to populate")

    (options, args) = parser.parse_args()

    if not options.inp or not options.out:
        parser.error("argument is missing")

    global ctx
    ctx.input_filenames = options.inp.split(":")
    ctx.output_filename = options.out


def writeln(txt):
    """ Write generated code to the output """
    global ctx
    ctx.output_file.write("%s%s\n" % ("\t" * ctx.identation_level, txt))


def ident(number):
    """ Increase or recrease identation for the writeln """
    global ctx
    ctx.identation_level += number


def gen_class(root_el):
    """ Parse a "class" element and generate the code. """

    name = root_el.get("name")

    # Write doxygen
    if not root_el.get("comment"):
        writeln("/// %s class." % name)
    else:
        writeln("/// %s." % root_el.get("comment"))

    # Body start
    writeln("class %s" % name)
    writeln("{")
    writeln("public:")

    # Parse members
    member_arr = []
    members_el = root_el.find("members")
    for member_el in members_el:
        member = MemberInfo()

        member.name = member_el.get("name")
        member.base_type = member_el.get("type")

        member.array_size = member_el.get("array_size")
        if not member.array_size:
            member.array_size = "1"

        if member_el.get("pointer") and member_el.get("pointer") == "true":
            member.pointer = True
        else:
            member.pointer = False

        if member_el.get("comment"):
            member.comment = member_el.get("comment")

        if member_el.get("constructor"):
            member.constructor = member_el.get("constructor")

        member_arr.append(member)

    # Write members
    ident(1)
    for member in member_arr:
        if member.comment:
            comment = "///< %s." % member.comment
        else:
            comment = ""

        if member.constructor:
            constructor = " %s" % member.constructor
        else:
            constructor = ""

        if member.pointer:
            writeln("%s* %s%s; %s" % (member.base_type, member.name, constructor, comment))
        elif member.array_size != "1":
            writeln("Array<%s, %s> %s%s; %s" % (member.base_type, member.array_size, member.name, constructor, comment))
        else:
            writeln("%s %s%s; %s" % (member.base_type, member.name, constructor, comment))
    ident(-1)

    # Before serialize make sure the dynamic arrays are last
    member_arr_copy = member_arr.copy()
    member_arr.sort(key=lambda x: x.is_dynamic_array(member_arr_copy))

    # Write the serialization and deserialization code
    writeln("")
    ident(1)
    writeln("template<typename TSerializer, typename TClass>")
    writeln("static void serializeCommon(TSerializer& s, TClass self)")
    writeln("{")
    ident(1)

    for member in member_arr:
        if member.is_pointer(member_arr_copy):
            writeln("s.doPointer(\"%s\", offsetof(%s, %s), self.%s);" % (member.name, name, member.name, member.name))
        elif member.is_dynamic_array(member_arr_copy):
            writeln("s.doDynamicArray(\"%s\", offsetof(%s, %s), self.%s, self.%s);" %
                    (member.name, name, member.name, member.name, member.array_size))
        elif member.array_size != "1":
            writeln("s.doArray(\"%s\", offsetof(%s, %s), &self.%s[0], self.%s.getSize());" %
                    (member.name, name, member.name, member.name, member.name))
        else:
            writeln("s.doValue(\"%s\", offsetof(%s, %s), self.%s);" % (member.name, name, member.name, member.name))

    ident(-1)
    writeln("}")
    ident(-1)

    # Write the methods
    writeln("")
    ident(1)
    writeln("template<typename TDeserializer>")
    writeln("void deserialize(TDeserializer& deserializer)")
    writeln("{")
    ident(1)
    writeln("serializeCommon<TDeserializer, %s&>(deserializer, *this);" % name)
    ident(-1)
    writeln("}")
    writeln("")

    writeln("template<typename TSerializer>")
    writeln("void serialize(TSerializer& serializer) const")
    writeln("{")
    ident(1)
    writeln("serializeCommon<TSerializer, const %s&>(serializer, *this);" % name)
    ident(-1)
    writeln("}")
    ident(-1)

    # Body end
    writeln("};")
    writeln("")


def gen_file(filename):
    """ Parse an XML file and generate the code. """

    tree = et.parse(filename)
    root = tree.getroot()

    for incs in root.iter("includes"):
        for inc in incs.iter("include"):
            writeln("#include %s" % inc.get("file"))

    writeln("")
    writeln("namespace anki")
    writeln("{")
    writeln("")

    doxygen_group_el = root.find("doxygen_group")
    if doxygen_group_el is not None:
        doxygen_group = doxygen_group_el.get("name")
        writeln("/// @addtogroup %s" % doxygen_group)
        writeln("/// @{")
        writeln("")

    prefix_code = root.find("prefix_code")
    if prefix_code is not None:
        writeln("%s" % prefix_code.text)

    for cls in root.iter("classes"):
        for cl in cls.iter("class"):
            gen_class(cl)

    if doxygen_group_el is not None:
        writeln("/// @}")
        writeln("")

    writeln("} // end namespace anki")


def main():
    parse_commandline()

    ctx.output_file = open(ctx.output_filename, "w")
    ctx.output_file.write("""// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

""")

    for filename in ctx.input_filenames:
        gen_file(filename)


if __name__ == "__main__":
    main()
