#!/usr/bin/python3

# Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import glob
import subprocess
import threading
import multiprocessing
import os
import tempfile
import shutil
import platform

file_extensions = ["h", "hpp", "c", "cpp", "glsl", "hlsl", "ankiprog"]
directories = ["AnKi", "Tests", "Sandbox", "Tools", "Samples"]
hlsl_semantics = ["TEXCOORD", "SV_POSITION", "SV_TARGET0",
                  "SV_TARGET", "SV_DISPATCHTHREADID", "SV_GROUPINDEX", "SV_GROUPID"]


def thread_callback(tid):
    """ Call clang-format """
    global mutex
    global file_names

    while True:
        mutex.acquire()
        if len(file_names) > 0:
            file_name = file_names.pop()
        else:
            file_name = None
        mutex.release()

        if file_name is None:
            break

        unused, file_extension = os.path.splitext(file_name)
        is_shader = file_extension == ".hlsl" or file_extension == ".ankiprog"

        if is_shader:
            # Read all text
            file = open(file_name, mode="r", newline="\n")
            file_txt = file.read()
            file.close()

            original_file_hash = hash(file_txt) 

            # Replace all semantics
            for semantic in hlsl_semantics:
                file_txt = file_txt.replace(": " + semantic, "__" + semantic)

            # Write the new file
            tmp_filefd, tmp_filename = tempfile.mkstemp()
            with open(tmp_filename, "w", newline="\n") as f:
                f.write(file_txt)
                os.close(tmp_filefd)

            orig_filename = file_name
            file_name = tmp_filename

            style_file = "--style=file:.clang-format-hlsl"
        else:
            style_file = "--style=file:.clang-format"

        if platform.system() == "Linux":
            exe = "./ThirdParty/Bin/Linux64/clang-format"
        else:
            exe = "./ThirdParty/Bin/Windows64/clang-format.exe"

        subprocess.check_call([exe, "-sort-includes=false", style_file, "-i", file_name])

        if is_shader:
            # Read tmp file
            file = open(tmp_filename, mode="r", newline="\n")
            file_txt = file.read()
            file.close()

            # Replace all semantics
            for semantic in hlsl_semantics:
                file_txt = file_txt.replace("__" + semantic, ": " + semantic)

            new_file_hash = hash(file_txt)

            # Write formatted file
            if new_file_hash != original_file_hash:
                file = open(orig_filename, mode="w", newline="\n")
                file.write(file_txt)
                file.close()

            # Cleanup
            os.remove(tmp_filename)

# Gather the filenames
file_names = []
for directory in directories:
    for extension in file_extensions:
        file_names.extend(glob.glob("./" + directory + "/**/*." + extension, recursive=True))
file_name_count = len(file_names)

# Start the threads
mutex = threading.Lock()
thread_count = multiprocessing.cpu_count()
threads = []
for i in range(0, thread_count):
    thread = threading.Thread(target=thread_callback, args=(i,))
    threads.append(thread)
    thread.start()

# Join the threads
for i in range(0, thread_count):
    threads[i].join()

print("Done! Formatted %d files" % file_name_count)
