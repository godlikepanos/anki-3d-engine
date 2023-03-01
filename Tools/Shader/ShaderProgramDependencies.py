#!/usr/bin/python3

# Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

# Generates the dependencies of a shader program file. Output consumed by CMake

import optparse
import re


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]",
                                   description="This script gathers all the include files of a shader program")

    parser.add_option("-i", "--input", dest="inp", type="string", help="Inpute .ankiprog file")

    (options, args) = parser.parse_args()

    if not options.inp:
        parser.error("argument is missing")

    return options.inp


def parse_file(fname):
    file = open(fname, mode="r")
    txt = file.read()

    my_includes = re.findall("\s*#\s*include\ <(.*)>", txt)
    agregated_includes = []

    for include in my_includes:
        # Skip non-shaders
        if "AnKi/Shaders" not in include:
            continue

        agregated_includes.append(include)

        # Append recursively
        other_includes = parse_file(include)
        for other_include in other_includes:
            if other_include not in agregated_includes:
                agregated_includes.append(other_include)

    return agregated_includes


def main():
    input_fname = parse_commandline()
    includes = parse_file(input_fname)

    str = ""
    for i in range(len(includes)):
        include = includes[i]
        include = include.replace("AnKi/Shaders/", "")

        if i < len(includes) - 1:
            str += include + ";"  # CMake wants ; as a separator
        else:
            str += include
    print(str, end="")  # No newline at the end


if __name__ == "__main__":
    main()
