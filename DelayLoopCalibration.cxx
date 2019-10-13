// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class DelayLoopCalibration.
//
// Copyright (C) 2019 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#include "sys.h"
#include "DelayLoopCalibration.h"
#include "debug.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <map>
#include <random>

#include "cwds/gnuplot_tools.h"

namespace utils {

//static
unsigned int const DelayLoopCalibrationBase::n = DelayLoopCalibrationBase::total_required_measurements();

unsigned int DelayLoopCalibrationBase::total_required_measurements()
{
  // If we do n measurements then the chance to get exactly i values that are non-outliers
  // is given by the probability mass function of the binomial distribution
  // (https://en.wikipedia.org/wiki/Binomial_distribution)
  //
  //   f(i, n, p) = {n choose i} p^{i} (1 - p)^{n - i}
  //
  // The chance that at least m values are non-outliers is
  //
  //   1 - F(m - 1, n, p).
  //
  // Namely, one minus the chance that at most m - 1 measurements are non-outliners.
  // Where F is the cumulative distribution function
  // (https://en.wikipedia.org/wiki/Binomial_distribution#Cumulative_distribution_function)
  //
  //   F(k; n, p) = \sum _{i=0}^{k} f(i, n, p),
  //
  // just summing up all values of f(i, n, p) for i = 0..k.
  //
  // We need to find an n >= m such that F(m - 1, n, p) becomes sufficiently small.

  // Using the https://en.wikipedia.org/wiki/Binomial_coefficient#Multiplicative_formula
  // for the binomial coefficient
  //
  // {n choose i} = \product _{j=1}^{i} {(n + 1 - j) / j}
  //
  // We can write
  //
  //   F(m - 1, n, p) = (1 - p)^{n} \sum _{i=0}^{m-1} { \product _{j=1}^{i} {p (n + 1 - j) / ((1 - p) j)}}
  //
  // Let, for a fixed n,
  //
  //   z_n(j) = p (n + 1 - j) / ((1 - p) j)
  //
  // and
  //
  //   F(m - 1, n, p) = (1 - p)^{n} S_n(m - 1),
  //
  // where
  //
  //   S_n(m) = \sum _{i=0}^{m} { \product _{j=1}^{i} {z_n(j)}} =
  //          = S_n(m - 1) + P_n(m), with S_n(-1) = 0.
  //
  // where
  //
  //   P_n(i) = \product _{j=1}^{i} {z_n(j)} =
  //          = P_n(i - 1) * z_n(i), with P_n(0) = 1.

  // Calculate (1 - p)^{n} = q^n for n = m.
  double q_exp_n = 1.0;
  for (unsigned int i = 0; i < m; ++i)
    q_exp_n *= 1 - p;

  // Find the smallest n >= m ...
  unsigned int n = m;
  for (;;)
  {
    double S_ni = 0.0;                          // S_n(-1)
    double P_ni = 1.0;                          // P_n(0).
    // Calculate S_n(i) for i = 0..m-1.
    unsigned int i = 0;
    for (;;)
    {
      // Calculate the next S_ni.
      S_ni += P_ni;                             // Calculate the value of S_n(i) = S_n(i - 1) + P_n(i).
      if (++i == m)
        break;
      // Calculate the next P_ni.
      P_ni *= p * (n + 1 - i) / ((1 - p) * i);  // Calculate the value of P_n(i) = P_n(i - 1) * z_n(i).
    }
    // Now S_ni == S_n(m - 1).
    if (S_ni * q_exp_n < epsilon)
      break;
    // Calculate the next q_exp_n.
    ++n;
    q_exp_n *= 1 - p;
  }

  return n;
}

double DelayLoopCalibrationBase::avg_of(unsigned int s)
{
  DoutEntering(dc::notice|flush_cf|continued_cf, "avg_of(" << s << ") = ");
  ASSERT(m <= n);
  std::vector<double> v;
  for (unsigned int i = 0; i <= n; ++i)
    v.push_back(measure(s));
  // Throw away the first measurement.
  v.erase(v.begin());
  std::sort(v.begin(), v.end());
  double sum = 0.0;
  for (unsigned int i = 0; i < m; ++i)
    sum += v[i];
  double result = sum / m;
  Dout(dc::finish, result);
  return result;
}

class LowestAverage
{
 private:
  std::vector<double> m_smallest_values;        // The std::min(m_max_to_average_over, m_total_number_of_measurements) number of (smallest) values that were added.
  double m_smallest_values_sum;                 // The sum of all the values in m_smallest_values.
  size_t m_total;                               // The number of times that add() was called.
  size_t const m_max_to_average_over;           // The maximum number of values stored in m_smallest_values.
  size_t const m_max_number_of_values;          // The (maximum) number of times that add() should be called (after that full() will return true).

 public:
  LowestAverage(unsigned int max_to_average_over, size_t max_number_of_values) :
    m_smallest_values_sum(0), m_total(0),
    m_max_to_average_over(max_to_average_over), m_max_number_of_values(max_number_of_values) { }

  double val() const { return m_smallest_values_sum / m_smallest_values.size(); }
  bool full() const { return m_total >= m_max_number_of_values; }

  void add(double v)
  {
    DoutEntering(dc::notice, "LowestAverage::add(" << v << ")");
    // Ignore measurements that are more than twice as large as the average value calculated so far.
    if (v * m_smallest_values.size() > 2 * m_smallest_values_sum)
      return;
    m_smallest_values.push_back(v);
    ++m_total;
    if (m_smallest_values.size() < m_max_to_average_over)
      m_smallest_values_sum += v;
    else
    {
      std::sort(m_smallest_values.begin(), m_smallest_values.end());
      m_smallest_values_sum -= m_smallest_values.back() - v;
      m_smallest_values.pop_back();
    }
    Dout(dc::notice, "Average now " << val() << " (" << m_total << " measurements)");
  }
};

unsigned int DelayLoopCalibrationBase::search_lowest_of(unsigned nm, double goal, unsigned int hint)
{
  DoutEntering(dc::notice, "DelayLoopCalibrationBase::search_lowest_of(" << nm << ", " << goal << ", " << hint << ")");
  constexpr int number_to_average_over = 10;
  std::map<unsigned int, LowestAverage> m_measurements;
  auto low = m_measurements.begin();
  auto low_s = [&](){
    return low == m_measurements.end() ? 0U : low->first;
  };
  auto low_delay = [&](){
    return low == m_measurements.end() ? 0.0 : low->second.val();
  };
  auto low_accuracy_reached = [&](){
    return low != m_measurements.end() && low->second.full();
  };
  auto high = [&](){
    return low == m_measurements.end() ? low : std::next(low);
  };
  auto high_s = [&](){
    auto high_iter = high();
    return high_iter == m_measurements.end() ? static_cast<unsigned int>(-1) : high_iter->first;
  };
  auto high_delay = [&](){
    ASSERT(high() != m_measurements.end());
    return high()->second.val();
  };
  auto high_accuracy_reached = [&](){
    auto high_iter = high();
    return high_iter != m_measurements.end() && high_iter->second.full();
  };

  unsigned int s = hint;
  while (!low_accuracy_reached() || !high_accuracy_reached() || high_s() - low_s() > 1)
  {
    if (s == low_s() && low_accuracy_reached())
      ++s;
    if (s == high_s() && high_accuracy_reached())
      --s;
    // Sanity check.
    ASSERT(low_s() < high_s());
    ASSERT(low_s() <= s && s <= high_s());

    double delay_s = measure(s);
    Dout(dc::notice, "measure(" << s << ") = " << delay_s);
    auto si = m_measurements.find(s);
    if (si == m_measurements.end())
      si = m_measurements.emplace(s, LowestAverage(number_to_average_over, nm)).first;
    ASSERT(!si->second.full());
    si->second.add(delay_s);
    delay_s = si->second.val();

    if (delay_s < goal)
    {
      ASSERT(s >= low_s());
      Dout(dc::notice(low_s() != s), "low_s " << low_s() << " --> " << s);
      low = si;
    }
    else
    {
      ASSERT(s <= high_s());
      if (si == low)
      {
        ASSERT(low != m_measurements.begin());
        --low;
      }
    }

    if (high() == m_measurements.end())
      s = std::max(s + 1, s * 2);
    else
      s = std::round(low_s() + (high_s() - low_s()) * (goal - low_delay()) / (high_delay() - low_delay()));
  }
  Dout(dc::notice, "Returning " << s);
  return s;
}

struct Point
{
  unsigned int m_s;
  double m_a;
  double m_error;

  Point(unsigned int s, double a) : m_s(s), m_a(a), m_error(0) { }
  Point() = default;

  friend bool operator<(Point const& p1, Point const& p2)
  {
    return p1.m_error < p2.m_error;
  }
};

#ifdef CWDEBUG
#define GNUPLOT
//#define INPUT_FILE "interesting3.dat"
#endif

unsigned int DelayLoopCalibrationBase::ransac(double goal, bool final COMMA_CWDEBUG_ONLY(std::string title))
{
#ifdef GNUPLOT
  bool draw_graph = !title.empty();
  eda::Plot plot1(title, "Loop size", "Slope");
  eda::Plot plot2(title, "Loop size", "Delay");
#endif
#ifdef INPUT_FILE
  std::ifstream infile;
  infile.open(INPUT_FILE);
#endif
  std::vector<Point> data;

  // Using https://en.wikipedia.org/wiki/Simple_linear_regression#Simple_linear_regression_without_the_intercept_term_(single_regressor)
  // we need to keep track of the sum of x*y and the sum of x*x.
  double slope;
  double sum_sd = 0, sum_s2 = 0;

  // Approach a loop size that results in goal milliseconds of delay.
  unsigned int s = 1000;
  do
  {
#ifndef INPUT_FILE
    double delay = final ? avg_of(s) : measure(s);
#else
    double a;
    int z;
    infile >> s >> a >> z;
    double delay = s * a;
#endif
    data.emplace_back(s, delay / s);
    sum_sd += s * delay;
    sum_s2 += 1.0 * s * s;
    slope = sum_sd / sum_s2;
    s *= 2;
  }
  while (s * slope < goal);

#ifdef GNUPLOT
  std::ofstream file;
  if (draw_graph)
  {
    file.open("data.dat");
    std::stringstream ss;
    ss << std::setprecision(2) << slope;
    for (auto& p : data)
    {
      plot1.add_data_point(p.m_s, p.m_a, 0, ss.str().c_str());
      plot2.add_data_point(p.m_s, p.m_a * p.m_s, 0, ss.str().c_str());
      file << p.m_s << ' ' << p.m_a << ' ' << p.m_error << '\n';
    }
  }
#endif

#ifdef INPUT_FILE
  size_t data_points = data.size() + 64;
  data.clear();
  sum_sd = sum_s2 = 0;
  for (int i = 0; i < data_points; ++i)
  {
    double a;
    double err;
    infile >> s >> a >> err;
    double delay = s * a;
    data.emplace_back(s, delay / s);
    sum_sd += s * delay;
    sum_s2 += 1.0 * s * s;
  }
#else
  // From the least square fit guess the loop size needed for goal milliseconds
  // and take 1/64th of that.
  double const sg64 = goal / (64 * slope);
  // Do another 64 measurements over the range [1/64 goal ... goal].
  for (int step = 1; step <= 64; ++step)
  {
    s = std::round(step * sg64);
    double delay = final ? avg_of(s) : measure(s);
    data.emplace_back(s, delay / s);
    sum_sd += s * delay;
    sum_s2 += 1.0 * s * s;
  }
#endif

  Dout(dc::notice, "data.size() = " << data.size());

  bool done = false;
  int iteration;
  for (iteration = 0; !done; ++iteration)
  {
    // Iteratively improve the best approximation of a line with with slope slope.
    slope = sum_sd / sum_s2;

    // Calculate the error of each (remaining) point.
    for (auto& p : data)
    {
      double da = p.m_a - slope;
      p.m_error = da * da;
    }
    // Sort the data by error.
    std::sort(data.begin(), data.end());

#ifdef GNUPLOT
    std::stringstream label;
    if (draw_graph)
      label << "err" << iteration;
#endif

    done = true;
    double error_sum = 0;
    sum_sd = sum_s2 = 0;
    for (size_t i = 0; i < data.size(); ++i)
    {
      error_sum += data[i].m_error;
      double avg_error = error_sum / (i + 1);
      int limit = (i == data.size() - 1) ? 4 : 16;
      // Throw away all points with an error that is larger than a certain threshold (but at least 2%).
      if (data[i].m_error > std::max(limit * avg_error, slope * slope * 0.0004))
      {
#ifdef GNUPLOT
        if (draw_graph)
        {
          file << '\n';
          Dout(dc::notice, "Error exceeded for i = " << i << "; throwing away " << (data.size() - i) << " data points.");
          for (size_t j = i; j < data.size(); ++j)
          {
            plot1.add_data_point(data[j].m_s, data[j].m_a, 0, label.str().c_str());
            plot2.add_data_point(data[j].m_s, data[j].m_s * data[j].m_a, 0, label.str().c_str());
            file << data[j].m_s << ' ' << data[j].m_a << ' ' << data[j].m_error << '\n';
          }
        }
#endif
        Dout(dc::notice, "data resize from " << data.size() << " to " << i);
        data.resize(i);
        ASSERT(data.size() == i);
        done = false;
        break;
      }
      //Dout(dc::notice, i << ": err^2 = " << data[i].m_error << "; avg err^2 = " << avg_error);
      // Recalculate sum_sd and sum_s2 of the remaining data point in order to calculate the next slope.
      double s2 = 1.0 * data[i].m_s * data[i].m_s;
      sum_sd += s2 * data[i].m_a;
      sum_s2 += s2;
    }
    if (done)
    {
      // Count the number of data points that are more than 2% below the estimated average.
      int low = 0;
      for (size_t i = 0; i < data.size(); ++i)
      {
        if (data[i].m_a < 0.98 * slope)
          ++low;
      }
      if (low >= 8)
      {
#ifdef GNUPLOT
        if (draw_graph)
          file << '\n';
#endif
        done = false;
        sum_sd = sum_s2 = 0;
        int cut_count = 0;
        for (size_t i = 0; i < data.size(); ++i)
        {
          // Throw away all points larger than the current estimate.
          if (data[i].m_a > slope)
          {
#ifdef GNUPLOT
            if (draw_graph)
            {
              plot1.add_data_point(data[i].m_s, data[i].m_a, 0, "cut");
              plot2.add_data_point(data[i].m_s, data[i].m_a * data[i].m_s, 0, "cut");
              file << data[i].m_s << ' ' << data[i].m_a << ' ' << data[i].m_error << '\n';
            }
#endif
            std::swap(data[i], data.back());
            ++cut_count;
            continue;
          }
          // Recalculate sum_sd and sum_s2 of the remaining data point in order to calculate the next slope.
          double s2 = 1.0 * data[i].m_s * data[i].m_s;
          sum_sd += s2 * data[i].m_a;
          sum_s2 += s2;
        }
        data.resize(data.size() - cut_count);
      }
    }
  }

  slope = sum_sd / sum_s2;
  s = std::round(goal / slope);
  Dout(dc::notice, "slope = " << slope << "; loop size = " << s);

#ifdef GNUPLOT
  if (draw_graph)
  {
    file << '\n';
    Dout(dc::notice, "Adding final " << data.size() << " points at the end of the file.");
    for (size_t i = 0; i < data.size(); ++i)
    {
      plot1.add_data_point(data[i].m_s, data[i].m_a, 0, "step");
      plot2.add_data_point(data[i].m_s, data[i].m_s * data[i].m_a, 0, "step");
      file << data[i].m_s << ' ' << data[i].m_a << ' ' << data[i].m_error << '\n';
    }
    file.close();

    plot2.add("set key top left");
    plot1.add("set key font \",6\"");
    plot2.add("set key font \",6\"");
    std::stringstream ss2;
    ss2 << "f(x) = " << slope;
    plot1.add(ss2.str().c_str());
    ss2.str(std::string());
    ss2 << "g(x) = " << (0.95 * slope);
    plot1.add(ss2.str().c_str());
    ss2.str(std::string());
    ss2 << "h(x) = " << (1.05 * slope);
    plot1.add(ss2.str().c_str());
    plot1.function("f(x)");
    plot1.function("g(x)");
    plot1.function("h(x)");
    ss2.str(std::string());
    ss2 << "f(x) = " << slope << "*x";
    plot2.add(ss2.str().c_str());
    plot2.function("f(x)");
    plot1.show();
    plot2.show();
  }
#endif

  return s;
}

} // namespace utils
