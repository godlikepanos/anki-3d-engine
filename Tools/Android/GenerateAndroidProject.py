#!/usr/bin/python3

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
    to_replace_with = to_replace_with.replace("\\", "\\\\")
    with fileinput.FileInput(filename, inplace=True) as file:
        for line in file:
            print(line.replace(to_replace, to_replace_with), end="")


def main():
    """ The main """

    ctx = parse_commandline()
    this_script_dir = os.path.abspath(os.path.dirname(os.path.realpath(__file__)))

    # Copy dir
    project_dir = os.path.join(ctx.out_dir, "AndroidProject_%s" % ctx.target)
    if not os.path.isdir(project_dir):
        shutil.copytree(this_script_dir, project_dir)

    # RM the script
    try:
        os.remove(os.path.join(project_dir, "GenerateAndroidProject.py"))
    except OSError:
        pass

    # Create the assets dir structure
    assets_dir = os.path.join(project_dir, "assets")
    if not os.path.isdir(assets_dir):
        os.mkdir(assets_dir)
        os.mkdir(os.path.join(project_dir, "assets/AnKi/"))
        os.symlink(os.path.join(this_script_dir, "../../AnKi/Shaders"),
                   os.path.join(project_dir, "assets/AnKi/Shaders"))
        os.symlink(os.path.join(this_script_dir, "../../EngineAssets"),
                   os.path.join(project_dir, "assets/EngineAssets"))
        os.mkdir(os.path.join(project_dir, "assets/ThirdParty/"))
        os.symlink(os.path.join(this_script_dir, "../../ThirdParty/Fsr"),
                   os.path.join(project_dir, "assets/ThirdParty/Fsr"))
        os.symlink(ctx.asserts_dir, os.path.join(project_dir, "assets/Assets"))

    # Write the asset directory structure to a file
    dir_structure_file = open(os.path.join(assets_dir, "DirStructure.txt"), "w", newline="\n")
    for root, dirs, files in os.walk(assets_dir, followlinks=True):
        for f in files:
            if f.find("DirStructure.txt") >= 0:
                continue

            filename = os.path.join(root, f)
            filename = filename.replace(assets_dir, "")
            filename = filename.replace("\\", "/")
            if filename[0] == '/':
                filename = filename[1:]
            dir_structure_file.write("%s\n" % filename)
    dir_structure_file.close()

    # strings.xml
    replace_in_file(os.path.join(project_dir, "app/src/main/res/values/strings.xml"), "%APP_NAME%", ctx.target)

    # build.gradle
    build_gradle = os.path.join(project_dir, "app/build.gradle")
    replace_in_file(build_gradle, "%TARGET%", ctx.target)
    replace_in_file(build_gradle, "%PYTHON%", sys.executable)
    replace_in_file(build_gradle, "%CMAKE%", os.path.join(this_script_dir, "../../CMakeLists.txt"))

    # Manifest
    replace_in_file(os.path.join(project_dir, "app/src/main/AndroidManifest.xml"), "%TARGET%", ctx.target)

    # Done
    print("Generated project: %s" % project_dir)


if __name__ == "__main__":
    main()
