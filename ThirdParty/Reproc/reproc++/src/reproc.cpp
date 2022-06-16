#include <reproc++/reproc.hpp>

#include <reproc/reproc.h>

namespace reproc {

namespace signal {

const int kill = REPROC_SIGKILL;
const int terminate = REPROC_SIGTERM;

}

const milliseconds infinite = milliseconds(REPROC_INFINITE);
const milliseconds deadline = milliseconds(REPROC_DEADLINE);

static std::error_code error_code_from(int r)
{
  if (r >= 0) {
    return {};
  }

  if (r == REPROC_EPIPE) {
    // https://github.com/microsoft/STL/pull/406
    return { static_cast<int>(std::errc::broken_pipe),
             std::generic_category() };
  }

  return { -r, std::system_category() };
}

static reproc_stop_actions reproc_stop_actions_from(stop_actions stop)
{
  return {
    { static_cast<REPROC_STOP>(stop.first.action), stop.first.timeout.count() },
    { static_cast<REPROC_STOP>(stop.second.action),
      stop.second.timeout.count() },
    { static_cast<REPROC_STOP>(stop.third.action), stop.third.timeout.count() }
  };
}

static reproc_redirect reproc_redirect_from(redirect redirect)
{
  return { static_cast<REPROC_REDIRECT>(redirect.type), redirect.handle,
           redirect.file, redirect.path };
}

static reproc_options reproc_options_from(const options &options, bool fork)
{
  return {
    options.working_directory,
    { static_cast<REPROC_ENV>(options.env.behavior), options.env.extra.data() },
    { reproc_redirect_from(options.redirect.in),
      reproc_redirect_from(options.redirect.out),
      reproc_redirect_from(options.redirect.err), options.redirect.parent,
      options.redirect.discard, options.redirect.file, options.redirect.path },
    reproc_stop_actions_from(options.stop),
    options.deadline.count(),
    { options.input.data(), options.input.size() },
    options.nonblocking,
    fork
  };
}

process::process() : impl_(reproc_new(), reproc_destroy) {}
process::~process() noexcept = default;

process::process(process &&other) noexcept = default;
process &process::operator=(process &&other) noexcept = default;

std::error_code process::start(const arguments &arguments,
                               const options &options) noexcept
{
  reproc_options reproc_options = reproc_options_from(options, false);
  int r = reproc_start(impl_.get(), arguments.data(), reproc_options);
  return error_code_from(r);
}

std::pair<bool, std::error_code> process::fork(const options &options) noexcept
{
  reproc_options reproc_options = reproc_options_from(options, true);
  int r = reproc_start(impl_.get(), nullptr, reproc_options);
  return { r == 0, error_code_from(r) };
}

std::pair<int, std::error_code> process::poll(int interests,
                                              milliseconds timeout)
{
  event::source source{ *this, interests, 0 };
  std::error_code ec = ::reproc::poll(&source, 1, timeout);
  return { source.events, ec };
}

std::pair<size_t, std::error_code>
process::read(stream stream, uint8_t *buffer, size_t size) noexcept
{
  int r = reproc_read(impl_.get(), static_cast<REPROC_STREAM>(stream), buffer,
                      size);
  return { r, error_code_from(r) };
}

std::pair<size_t, std::error_code> process::write(const uint8_t *buffer,
                                                  size_t size) noexcept
{
  int r = reproc_write(impl_.get(), buffer, size);
  return { r, error_code_from(r) };
}

std::error_code process::close(stream stream) noexcept
{
  int r = reproc_close(impl_.get(), static_cast<REPROC_STREAM>(stream));
  return error_code_from(r);
}

std::pair<int, std::error_code> process::wait(milliseconds timeout) noexcept
{
  int r = reproc_wait(impl_.get(), timeout.count());
  return { r, error_code_from(r) };
}

std::error_code process::terminate() noexcept
{
  int r = reproc_terminate(impl_.get());
  return error_code_from(r);
}

std::error_code process::kill() noexcept
{
  int r = reproc_kill(impl_.get());
  return error_code_from(r);
}

std::pair<int, std::error_code> process::stop(stop_actions stop) noexcept
{
  int r = reproc_stop(impl_.get(), reproc_stop_actions_from(stop));
  return { r, error_code_from(r) };
}

std::pair<int, std::error_code> process::pid() noexcept
{
  int r = reproc_pid(impl_.get());
  return { r, error_code_from(r) };
}

std::error_code
poll(event::source *sources, size_t num_sources, milliseconds timeout)
{
  auto *reproc_sources = new reproc_event_source[num_sources];

  for (size_t i = 0; i < num_sources; i++) {
    reproc_sources[i] = { sources[i].process.impl_.get(), sources[i].interests,
                          0 };
  }

  int r = reproc_poll(reproc_sources, num_sources, timeout.count());

  if (r >= 0) {
    for (size_t i = 0; i < num_sources; i++) {
      sources[i].events = reproc_sources[i].events;
    }
  }

  delete[] reproc_sources;

  return error_code_from(r);
}

}
