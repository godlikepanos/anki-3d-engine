#include <iostream>

#include <reproc++/run.hpp>

// Equivalent to reproc's run example but implemented using reproc++.
int main(int argc, const char **argv)
{
  (void) argc;

  int status = -1;
  std::error_code ec;

  reproc::options options;
  options.redirect.parent = true;
  options.deadline = reproc::milliseconds(5000);

  std::tie(status, ec) = reproc::run(argv + 1, options);

  if (ec) {
    std::cerr << ec.message() << std::endl;
  }

  return ec ? ec.value() : status;
}
