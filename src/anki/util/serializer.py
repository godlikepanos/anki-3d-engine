#!/usr/bin/python3

# Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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
    __slots__ = ["name", "base_type", "array_size", "comment", "pointer"]

    def __init__(self):
        self.name = None
        self.base_type = None
        self.array_size = 1
        self.comment = None
        self.pointer = False


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]", description="Create serialization code using XML")

    parser.add_option(
        "-i", "--input", dest="inp", type="string", help="specify the XML files to parse. Seperate with :")

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
            member.array_size = 1
        elif str(member.array_size).isdigit():
            member.array_size = int(member.array_size)

        if member_el.get("pointer") and member_el.get("pointer") == "true":
            member.pointer = True
        else:
            member.pointer = False

        if member_el.get("comment"):
            member.comment = member_el.get("comment")

        member_arr.append(member)

    # Write members
    ident(1)
    for member in member_arr:
        if member.comment:
            comment = "///< %s." % member.comment
        else:
            comment = ""

        if member.pointer:
            writeln("%s* %s; %s" % (member.base_type, member.name, comment))
        elif member.array_size > 1:
            writeln("Array<%s, %d> %s; %s" % (member.base_type, member.array_size, member.name, comment))
        else:
            writeln("%s %s; %s" % (member.base_type, member.name, comment))
    ident(-1)

    # Write the serializer code
    writeln("")
    ident(1)
    writeln("template<typename TSerializer>")
    writeln("void serialize(TSerializer& serializer) const")
    writeln("{")
    ident(1)

    for member in member_arr:
        if member.pointer and str(member.array_size) == 1:
            writeln("serializer.writePointer(%s);" % member.name)
        elif member.pointer:
            writeln("serializer.writeDynamicArray(%s, %s);" % (member.name, member.array_size))
        elif member.array_size > 1:
            writeln("serializer.writeArray(&%s[0], %d);" % (member.name, member.array_size))
        else:
            writeln("serializer.writeValue(%s);" % member.name)

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

    for cls in root.iter("classes"):
        for cl in cls.iter("class"):
            gen_class(cl)

    writeln("} // end namespace anki")


def main():
    parse_commandline()

    ctx.output_file = open(ctx.output_filename, "w")
    ctx.output_file.write("""// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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
