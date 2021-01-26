#include "CommandLine.h"

static std::string_view basename_nonModifying(char* path) noexcept;

CommandLine::CommandLine(int _argc, char* _argv[]) noexcept
  : args(_argc, _argv),
    argc(_argc),
    argv(_argv) {
  if (_argv[0] != nullptr && _argv[0][0] != '\0') {
    programName = _argv[0];
    baseProgramName = basename_nonModifying(_argv[0]);
  }
}
CommandLine::Args::Args(int argc, char** argv) noexcept
  : m_size(argc == 0 ? 0 : argc - 1),
    m_vector(argv) {
}
int CommandLine::Args::size() const noexcept {
  return m_size;
}
std::string_view CommandLine::Args::operator[](int idx) const {
  return m_vector[1 + idx];
}
static std::string_view basename_nonModifying(char* _path) noexcept {
  static char dot[] = ".";

  if (_path == nullptr) {
    return dot;
  }
  std::string_view path(_path);
  if (path.size() == 0) {
    return dot;
  }
  const auto end_pos = path.find_last_not_of("/");
  if (end_pos == 0 || end_pos == std::string_view::npos) {
    return path.substr(0,1);
  }
  auto start_pos = path.find_last_of("/", end_pos - 1);
  start_pos = (start_pos == std::string_view::npos) ? 0 : start_pos + 1;
  return path.substr(start_pos, end_pos - start_pos + 1);
}
