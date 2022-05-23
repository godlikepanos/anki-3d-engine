#!/usr/bin/python3

# Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
    asserts_dir = None
    out_dir = ""
    shader_compiler = ""


def parse_commandline():
    """ Parse the command line arguments """

    parser = optparse.OptionParser(usage="usage: %prog [options]", description="Generate an Android gradle project")

    parser.add_option("-o", "--out-dir", dest="out_dir", type="string", help="Where to create the project")
    parser.add_option("-t", "--target", dest="target", type="string", help="The name of .so to package")
    parser.add_option("-a", "--assets", dest="assets", type="string", help="Assets directory")
    parser.add_option("-c",
                      "--shader-compiler",
                      dest="shader_compiler",
                      type="string",
                      help="The path of the shader compiler")

    (options, args) = parser.parse_args()

    required = "target out_dir shader_compiler".split()
    for r in required:
        if options.__dict__[r] is None:
            parser.print_help()
            parser.error("parameter \"%s\" required" % r)

    ctx = Context()
    ctx.target = options.target
    if options.assets is not None:
        ctx.asserts_dir = os.path.abspath(options.assets)
    ctx.out_dir = os.path.abspath(options.out_dir)
    ctx.shader_compiler = os.path.abspath(options.shader_compiler)

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

    # Copy template
    project_dir = os.path.join(ctx.out_dir, "AndroidProject_%s" % ctx.target)
    if not os.path.isdir(project_dir):
        shutil.copytree(this_script_dir, project_dir)
    else:
        print("Project directory (%s) already exists. Won't copy template" % project_dir)

    # RM the script
    try:
        os.remove(os.path.join(project_dir, "GenerateAndroidProject.py"))
    except OSError:
        pass

    # Create the assets dir structure
    assets_dir = os.path.join(project_dir, "assets")
    if not os.path.isdir(assets_dir):
        os.mkdir(assets_dir)
        os.symlink(os.path.join(this_script_dir, "../../EngineAssets"),
                   os.path.join(project_dir, "assets/EngineAssets"))
        if ctx.asserts_dir is not None:
            os.symlink(ctx.asserts_dir, os.path.join(project_dir, "assets/Assets"))
    else:
        print("Asset directory (%s) already exists. Skipping" % assets_dir)

    # strings.xml
    replace_in_file(os.path.join(project_dir, "app/src/main/res/values/strings.xml"), "%APP_NAME%", ctx.target)

    # build.gradle
    build_gradle = os.path.join(project_dir, "app/build.gradle")
    replace_in_file(build_gradle, "%TARGET%", ctx.target)
    replace_in_file(build_gradle, "%PYTHON%", sys.executable)
    replace_in_file(build_gradle, "%COMPILER%", ctx.shader_compiler)
    replace_in_file(build_gradle, "%CMAKE%", os.path.join(this_script_dir, "../../CMakeLists.txt"))

    # Manifest
    replace_in_file(os.path.join(project_dir, "app/src/main/AndroidManifest.xml"), "%TARGET%", ctx.target)

    # Done
    print("Generated project: %s" % project_dir)


if __name__ == "__main__":
    main()
