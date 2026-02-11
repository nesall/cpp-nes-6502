#include "nesdefs.hpp"

#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <stdexcept>


bool cppnes::Toolchain::isValid()
{
  return std::filesystem::exists(ca65path_) && std::filesystem::exists(ld65path_);
}

void cppnes::Toolchain::setCa65(std::string_view path)
{
  ca65path_ = path;
}

void cppnes::Toolchain::setLd65(std::string_view path)
{
  ld65path_ = path;
}

void cppnes::Toolchain::compile(const std::filesystem::path &asmFile, const std::filesystem::path &objFile)
{
  // example call: ca65 example.s --verbose --target nes -g -o example.o
  if (!isValid())
    throw std::runtime_error("Toolchain not configured");
  if (!std::filesystem::exists(asmFile))
    throw std::runtime_error("ASM file does not exist");
  std::ostringstream cmd;
  cmd << '"' << ca65path_ << '"'
    << " \"" << asmFile.string() << "\""
    << " --target nes"
    << " -g"
    << " -o \"" << objFile.string() << '"';

  int rc = std::system(cmd.str().c_str());
  if (rc != 0)
    throw std::runtime_error("ca65 compilation failed");
}

void cppnes::Toolchain::link(const std::filesystem::path &cfgFile, const std::filesystem::path &objFile, std::string_view outputPath)
{
  if (!isValid())
    throw std::runtime_error("Toolchain not configured");

  if (!std::filesystem::exists(cfgFile))
    throw std::runtime_error("Linker config file does not exist");

  if (!std::filesystem::exists(objFile))
    throw std::runtime_error("Object file does not exist");

  std::ostringstream cmd;
  cmd << '"' << ld65path_ << '"'
    << " -C \"" << cfgFile.string() << '"'
    << " \"" << objFile.string() << '"'
    << " -o \"" << outputPath << '"';

  int rc = std::system(cmd.str().c_str());
  if (rc != 0)
    throw std::runtime_error("ld65 linking failed");
}
