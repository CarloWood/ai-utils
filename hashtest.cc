#include <random>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <array>

void plot(std::array<long, 65> const& histogram, size_t max_count)
{
  long step = max_count / 40;
  for (long m = max_count; m >= 0; m -= step)
  {
    for (int s = 0; s <= 64; ++s)
    {
      if (histogram[s] >= m)
        std::cout << '#';
      else
        std::cout << ' ';
    }
    std::cout << '\n';
  }
  for (int s = 0; s <= 64; ++s)
    std::cout << (s % 10);
  std::cout << std::endl;
}

int main()
{
  std::random_device random_device;
  std::mt19937 generator(random_device());
  std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

  int const number_of_inputs = 200;
  int const number_of_hashes = number_of_inputs * (number_of_inputs - 1);

  std::vector<uint64_t> v;
  for (int i = 0; i < number_of_hashes; ++i)
    v.push_back(dist(generator));

  std::cout << "Comparing " << (v.size() * (v.size() - 1)) << " pairs of numbers.\n";

  std::array<long, 65> histogram = {};
  size_t max_count = 0;
  size_t prs = 0;
  int max_same = 0;
  for (auto h1 : v)
  {
    for (auto h2 : v)
    {
      if (h1 != h2)
      {
        ++prs;
        auto equal = ~h1 ^ h2;
        int same = __builtin_popcountll(equal);
        if (same > max_same)
          max_same = same;
        histogram[same] += 1;
        if (histogram[same] > max_count)
          max_count = histogram[same];
      }
    }
  }
  std::cout << "max_same = " << max_same << "; max_count = " << max_count << std::endl;

  plot(histogram, max_count);

  for (int i = 0; i < histogram.size(); ++i)
    histogram[i] = 0;

  max_count = 0;
  max_same = 0;
  for (int i = 0; i < prs; ++i)
  {
    uint64_t h1 = dist(generator);
    uint64_t h2 = dist(generator);
    auto equal = ~h1 ^ h2;
    int same = __builtin_popcountll(equal);
    if (same > max_same)
      max_same = same;
    histogram[same] += 1;
    if (histogram[same] > max_count)
      max_count = histogram[same];
  }
  std::cout << "max_same = " << max_same << "; max_count = " << max_count << std::endl;

  plot(histogram, max_count);
}
