#pragma once
#include <string_view>

// View object presenting a program's command line.
// Its purpose is to be easier to work with than argc/argv.
class CommandLine {
public:
  class Args {
  public:
    Args(int argc, char** argv) noexcept;
    int size() const noexcept;
    std::string_view operator[](int) const;
  private:
    int m_size;
    char** m_vector;
  };
public:
  CommandLine(int argc, char* argv[]) noexcept;
  std::string_view programName;
  std::string_view baseProgramName;
  Args args;
  int argc;
  char** argv;
private:

};
