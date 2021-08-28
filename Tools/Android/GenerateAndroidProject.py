#!/usr/bin/python

# Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import os
import optparse
import shutil
import fileinput
import sys


class Context:
    target = ""
    asserts_dir = ""
    out_dir = ""


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]", description="Generate an Android gradle project")

    parser.add_option("-o", "--out-dir", dest="out_dir", type="string", help="Where to create the project")
    parser.add_option("-t", "--target", dest="target", type="string", help="The name of .so to package")
    parser.add_option("-a", "--assets", dest="assets", type="string", help="Assets directory")

    (options, args) = parser.parse_args()

    required = "target assets out_dir".split()
    for r in required:
        if options.__dict__[r] is None:
            parser.print_help()
            parser.error("parameter \"%s\" required" % r)

    ctx = Context()
    ctx.target = options.target
    ctx.asserts_dir = os.path.abspath(options.assets)
    ctx.out_dir = os.path.abspath(options.out_dir)

    return ctx


def replace_in_file(filename, to_replace, to_replace_with):
    with fileinput.FileInput(filename, inplace=True) as file:
        for line in file:
            print(line.replace(to_replace, to_replace_with), end="")


def main():
    """ The main """

    ctx = parse_commandline()

    # Copy dir
    project_dir = os.path.join(ctx.out_dir, "AndroidProject_%s" % ctx.target)
    if os.path.isdir(project_dir):
        raise Exception("Directory already exists: %s" % project_dir)

    this_script_dir = os.path.abspath(os.path.dirname(os.path.realpath(__file__)))
    shutil.copytree(this_script_dir, project_dir)

    # RM the script
    os.remove(os.path.join(project_dir, "GenerateAndroidProject.py"))

    # Create the assets dir structure
    assets_dir = os.path.join(project_dir, "assets")
    os.mkdir(assets_dir)
    os.mkdir(os.path.join(project_dir, "assets/AnKi/"))
    os.symlink(os.path.join(this_script_dir, "../../AnKi/Shaders"), os.path.join(project_dir, "assets/AnKi/Shaders"))
    os.symlink(os.path.join(this_script_dir, "../../EngineAssets"), os.path.join(project_dir, "assets/EngineAssets"))
    os.symlink(ctx.asserts_dir, os.path.join(project_dir, "assets/Assets"))

    # strings.xml
    replace_in_file(os.path.join(project_dir, "app/src/main/res/values/strings.xml"), "%APP_NAME%", ctx.target)

    # build.gradle
    build_gradle = os.path.join(project_dir, "app/build.gradle")
    replace_in_file(build_gradle, "%TARGET%", ctx.target)
    replace_in_file(build_gradle, "%PYTHON%", sys.executable)
    replace_in_file(build_gradle, "%CMAKE%", os.path.join(this_script_dir, "../../CMakeLists.txt"))

    # Done
    print("Generated project: %s" % project_dir)


if __name__ == "__main__":
    main()
