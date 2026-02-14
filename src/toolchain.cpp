#include "nesdefs.hpp"
#include <filesystem>
#include <stdexcept>
#include "3rdparty/utils_log/logger.hpp"
#include "3rdparty/procmngr.h"


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

void cppnes::Toolchain::compile(const std::filesystem::path &asmFile,  const std::filesystem::path &objFile)
{
  if (!isValid())
    throw std::runtime_error("Toolchain not configured");

  if (!std::filesystem::exists(asmFile))
    throw std::runtime_error("ASM file does not exist");

  ProcessManager pm;

  std::vector<std::string> args = {
      asmFile.string(),
      "-g",
      "-o",
      objFile.string()
  };

  if (!pm.startProcess(ca65path_.string(), args))
    throw std::runtime_error("Failed to start ca65");

  if (!pm.waitForCompletion())
    throw std::runtime_error("ca65 timeout");

  if (pm.getExitCode() != 0)
    throw std::runtime_error("ca65 compilation failed");

  LOG_MSG << "ca65 compilation complete.";
}

void cppnes::Toolchain::link(const std::filesystem::path &cfgFile, const std::filesystem::path &objFile, std::filesystem::path outputPath)
{
  if (!isValid())
    throw std::runtime_error("Toolchain not configured");

  if (!std::filesystem::exists(cfgFile))
    throw std::runtime_error("Linker config file does not exist");

  if (!std::filesystem::exists(objFile))
    throw std::runtime_error("Object file does not exist");

  ProcessManager pm;

  std::vector<std::string> args = {
    objFile.string(),
    "-C",
    cfgFile.string(),
    "--dbgfile",
    (outputPath.parent_path() / "prg.dbg").string(),
    "-o",
    outputPath.string()
  };

  if (!pm.startProcess(ld65path_.string(), args))
    throw std::runtime_error("Failed to start ld65");

  if (!pm.waitForCompletion())
    throw std::runtime_error("ld65 timeout");

  if (pm.getExitCode() != 0)
    throw std::runtime_error("ld65 compilation failed");

  LOG_MSG << "ld65 linking complete.";
}
