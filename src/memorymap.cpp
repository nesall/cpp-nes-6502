#include "nesdefs.hpp"
#include <stdexcept>
#include <cassert>

cppnes::ZpAddress cppnes::ZeroPageAllocator::alloc(std::string_view name, bool constant)
{
  if (next_ > Max)
    throw std::out_of_range("ZeroPageAllocator: zero page exhausted");

  uint16_t addr = next_++;
  return ZpAddress::fromValue(addr, name, constant);
}


cppnes::AbsAddress cppnes::RamAllocator::alloc(std::string_view name, bool constant)
{
  return allocBlock(name, 1, 0, constant);
}

cppnes::AbsAddress cppnes::RamAllocator::allocBlock(std::string_view name, uint16_t size, uint16_t baseAddress, bool constant)
{
  assert(0 < size);
  if (size == 0)
    throw std::logic_error("allocBlock: size must be > 0");

  uint16_t base;

  if (baseAddress != 0) {
    // Explicit placement
    if (baseAddress < Min || Max < baseAddress)
      throw std::out_of_range("allocBlock: baseAddress outside RAM");

    if (Max < baseAddress + size - 1)
      throw std::out_of_range("allocBlock: block exceeds RAM");

    base = baseAddress;
  } else {
    // Automatic placement
    if (Max < next_ + size - 1)
      throw std::out_of_range("allocBlock: RAM exhausted");

    base = next_;
    next_ += size;
  }

  return AbsAddress{ base, name, constant };
}