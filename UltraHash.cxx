#include "sys.h"
#include "UltraHash.h"
#include <random>

namespace utils {

int UltraHash::initialize(std::vector<uint64_t> const& keys)
{
  std::mt19937_64 gen64;

  m_b[0] = 0x800000;
  for (auto& s : m_sets)
    for (auto& e : s)
      e = gen64();
}

} // namespace utils
