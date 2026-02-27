#include "nesdefs.hpp"
#include <filesystem>
#include <fstream>

void cppnes::Resources::loadCHR(std::string_view path)
{
  std::filesystem::path p{ path };
  if (!std::filesystem::exists(p))
    throw std::runtime_error("CHR file does not exist: " + p.string());

  std::ifstream file(p, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    throw std::runtime_error("Failed to open CHR file: " + p.string());

  auto size = file.tellg();
  if (size <= 0 || 8192 < size) // NES CHR is max 8 KB
    throw std::runtime_error("CHR file size invalid (0–8192 bytes expected)");

  chrData_.resize(static_cast<size_t>(size));
  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(chrData_.data()), size);
  if (!file)
    throw std::runtime_error("Failed to read complete CHR file: " + p.string());
  chrPath_ = path;
}

void cppnes::Resources::loadPalettes(std::string_view path)
{
  // TODO:
}

void cppnes::Resources::setPalettes(std::array<uint8_t, 16> bg, std::array<uint8_t, 16> spr)
{
  bgPal_ = bg;
  spPal_ = spr;
}

void cppnes::Resources::addNametable(std::string_view label, std::string_view filename)
{
  nametables_[std::string(label)] = std::string(filename);
}