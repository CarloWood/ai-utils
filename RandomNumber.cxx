#include "sys.h"
#include "RandomNumber.h"
#include <chrono>
#include "debug.h"

namespace utils {

RandomNumber::RandomNumber()
{
  DoutEntering(dc::notice, "RandomNumber::RandomNumber() [" << (void*)this << "]");
  std::random_device rd;
  result_type seed_value = rd() ^ (
          (result_type)
          std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now().time_since_epoch()
              ).count() +
          (result_type)
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch()
              ).count() );
  Dout(dc::notice, "seed used is " << std::hex << seed_value);
  seed(seed_value);
}

} // namespace utils
