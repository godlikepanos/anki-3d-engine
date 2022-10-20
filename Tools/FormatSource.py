#!/usr/bin/python3

# Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
# All rights reserved.
# Code licensed under the BSD License.
# http://www.anki3d.org/LICENSE

import glob
import subprocess
import threading
import multiprocessing

file_extensions = ["h", "hpp", "c", "cpp", "glsl", "ankiprog"]
directories = ["AnKi", "Tests", "Sandbox", "Tools", "Samples"]


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

        subprocess.check_call(["./ThirdParty/Bin/Windows64/clang-format.exe", "-sort-includes=false", "-i", file_name])


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
