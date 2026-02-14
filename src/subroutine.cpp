#include "nesdefs.hpp"
#include "nesdefs_helper.hpp"

cppnes::Subroutine::Subroutine(std::string_view name) : name_(name)
{
}

cppnes::SubroutineBblocksProxy cppnes::Subroutine::bblocks()
{
  return SubroutineBblocksProxy(*this);
}