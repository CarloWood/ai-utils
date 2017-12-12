#include "sys.h"
#include "Vector.h"
#include "debug.h"
#include <iostream>

namespace utils {

std::ostream& operator<<(std::ostream& os, VectorIndex const& index)
{
  os << '#' << index.m_value;
  return os;
}

} // namespace utils
