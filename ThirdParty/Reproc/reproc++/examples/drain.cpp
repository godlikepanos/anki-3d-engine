#include <array>
#include <iostream>

#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

static int fail(std::error_code ec)
{
  std::cerr << ec.message();
  return ec.value();
}

// Uses `reproc::drain` to show the output of the given command.
int main(int argc, const char **argv)
{
  if (argc <= 1) {
    std::cerr << "No arguments provided. Example usage: "
              << "./drain cmake --help";
    return EXIT_FAILURE;
  }

  reproc::process process;

  // reproc++ uses error codes to report errors. If exceptions are preferred,
  // convert `std::error_code`'s to exceptions using `std::system_error`.
  std::error_code ec = process.start(argv + 1);

  // reproc++ converts system errors to `std::error_code`'s of the system
  // category. These can be matched against using values from the `std::errc`
  // error condition. See https://en.cppreference.com/w/cpp/error/errc for more
  // information.
  if (ec == std::errc::no_such_file_or_directory) {
    std::cerr << "Program not found. Make sure it's available from the PATH.";
    return ec.value();
  } else if (ec) {
    return fail(ec);
  }

  // `reproc::drain` reads from the stdout and stderr streams of `process` until
  // both are closed or an error occurs. Providing it with a string sink for a
  // specific stream makes it store all output of that stream in the string
  // passed to the string sink. Passing the same sink to both the `out` and
  // `err` arguments of `reproc::drain` causes the stdout and stderr output to
  // get stored in the same string.
  std::string output;
  reproc::sink::string sink(output);
  // By default, reproc only redirects stdout to a pipe and not stderr so we
  // pass `reproc::sink::null` as the sink for stderr here. We could also pass
  // `sink` but it wouldn't receive any data from stderr.
  ec = reproc::drain(process, sink, reproc::sink::null);
  if (ec) {
    return fail(ec);
  }

  std::cout << output << std::flush;

  // It's easy to define your own sinks as well. Take a look at `drain.hpp` in
  // the repository to see how `sink::string` and other sinks are implemented.
  // The documentation of `reproc::drain` also provides more information on the
  // requirements a sink should fulfill.

  // By default, The `process` destructor waits indefinitely for the child
  // process to exit to ensure proper cleanup. See the forward example for
  // information on how this can be configured. However, when relying on the
  // `process` destructor, we cannot check the exit status of the process so it
  // usually makes sense to explicitly wait for the process to exit and check
  // its exit status.

  int status = 0;
  std::tie(status, ec) = process.wait(reproc::infinite);
  if (ec) {
    return fail(ec);
  }

  return status;
}
