#include <iostream>

#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

static int fail(std::error_code ec)
{
  std::cerr << ec.message();
  return ec.value();
}

// The forward example forwards the program arguments to a child process and
// prints its output on stdout.
//
// Example: "./forward cmake --help" will print CMake's help output.
//
// This program can be used to verify that manually executing a command and
// executing it using reproc produces the same output.
int main(int argc, const char **argv)
{
  if (argc <= 1) {
    std::cerr << "No arguments provided. Example usage: ./forward cmake --help";
    return EXIT_FAILURE;
  }

  reproc::process process;

  // Stop actions can be passed to both `process::start` (via `options`) and
  // `process::stop`. Stop actions passed to `process::start` are passed to
  // `process::stop` in the `process` destructor. This can be used to make sure
  // that a child process is always stopped correctly when its corresponding
  // `process` instance is destroyed.
  //
  // Any program can be started with forward so we make sure the child process
  // is cleaned up correctly by specifying `reproc::terminate` which sends
  // `SIGTERM` (POSIX) or `CTRL-BREAK` (Windows) and waits five seconds. We also
  // add the `reproc::kill` flag which sends `SIGKILL` (POSIX) or calls
  // `TerminateProcess` (Windows) if the process hasn't exited after five
  // seconds and waits two more seconds for the child process to exit.
  //
  // If the `stop_actions` struct passed to `process::start` is
  // default-initialized, the `process` destructor will wait indefinitely for
  // the child process to exit.
  //
  // Note that C++14 has chrono literals which allows
  // `reproc::milliseconds(5000)` to be replaced with `5000ms`.
  reproc::stop_actions stop = {
    { reproc::stop::noop, reproc::milliseconds(0) },
    { reproc::stop::terminate, reproc::milliseconds(5000) },
    { reproc::stop::kill, reproc::milliseconds(2000) }
  };

  reproc::options options;
  options.stop = stop;

  // We have the child process inherit the parent's standard streams so the
  // child process reads directly from the stdin and writes directly to the
  // stdout/stderr of the parent process.
  options.redirect.parent = true;

  // Exclude `argv[0]` which is the current program's name.
  std::error_code ec = process.start(argv + 1, options);

  if (ec == std::errc::no_such_file_or_directory) {
    std::cerr << "Program not found. Make sure it's available from the PATH.";
    return ec.value();
  } else if (ec) {
    return fail(ec);
  }

  // Call `process::stop` manually so we can access the exit status. We add
  // `reproc::wait` with a timeout of ten seconds to give the process time to
  // exit on its own before sending `SIGTERM`.
  options.stop.first = { reproc::stop::wait, reproc::milliseconds(10000) };

  int status = 0;
  std::tie(status, ec) = process.stop(options.stop);
  if (ec) {
    return fail(ec);
  }

  return status;
}
