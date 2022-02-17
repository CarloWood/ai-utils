/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class DelayLoopCalibration.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sys.h"
#include "DelayLoopCalibration.h"
#include "debug.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <map>
#include <random>
#include <thread>
#include <sched.h>

#ifdef CWDEBUG
//#define GNUPLOT
//#define INPUT_FILE "beauty.out"
//#define OUTPUT_FILE "measure.out"
#endif

#ifdef GNUPLOT
#include "cwds/gnuplot_tools.h"
#endif

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
channel_ct delayloop("DELAYLOOP");
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {

namespace {
DelayLoopCalibrationBase::GlobalTotalRequiredMeasurements n_dummy;
} // namespace

unsigned int DelayLoopCalibrationBase::TotalRequiredMeasurements::total_required_measurements() const
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
  DoutEntering(dc::delayloop|flush_cf|continued_cf, "avg_of(" << s << ") = ");
  unsigned int const n = GlobalTotalRequiredMeasurements::instantiate();
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
    DoutEntering(dc::delayloop, "LowestAverage::add(" << v << ")");
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
    Dout(dc::delayloop, "Average now " << val() << " (" << m_total << " measurements)");
  }
};

unsigned int DelayLoopCalibrationBase::search_lowest_of(unsigned nm, double goal, unsigned int hint)
{
  DoutEntering(dc::delayloop, "DelayLoopCalibrationBase::search_lowest_of(" << nm << ", " << goal << ", " << hint << ")");
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
    Dout(dc::delayloop, "measure(" << s << ") = " << delay_s);
    auto si = m_measurements.find(s);
    if (si == m_measurements.end())
      si = m_measurements.emplace(s, LowestAverage(number_to_average_over, nm)).first;
    ASSERT(!si->second.full());
    si->second.add(delay_s);
    delay_s = si->second.val();

    if (delay_s < goal)
    {
      ASSERT(s >= low_s());
      Dout(dc::delayloop(low_s() != s), "low_s " << low_s() << " --> " << s);
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
  Dout(dc::delayloop, "Returning " << s);
  return s;
}

// Abbreviations used:
//
// s  : size of the delay loop.
// a  : slope of the fitted line (alpha).
// d  : delay (in ms).
// s2 : s^2.
// sd : s * d.
// ols: Ordinary Least Squares.
//
// We fit a line d = a * s (through the origin) by minimizing the sum of the squares of the vertical distances of each data point to that line.
//
// Aka (see https://en.wikipedia.org/wiki/Ordinary_least_squares) assuming that for each measurement i
//
//   d_i = a * s_i + e_i
//
// the value of a that minimizes \sum _{i=0}^N { e_i^{2} } is
//
//   a = \sum _{i=0}^N { d_i * s_i } / \sum _{i=0}^N { s_i * s_i }.

struct Point
{
 private:
  unsigned int m_s;
  double m_a;

 public:
  double m_s2;
  double m_sd;

  static constexpr int max_number_of_points_per_fit = 8;
  std::array<double, max_number_of_points_per_fit> m_sum_s2;    // The index is the number of point in this fit minus one.
  std::array<double, max_number_of_points_per_fit> m_sum_sd;
  std::array<double, max_number_of_points_per_fit> m_ols_a;
  std::array<double, max_number_of_points_per_fit> m_ols_e2;

  unsigned int loop_size() const { return m_s; }
  double delay() const { return m_s * m_a; }
  double slope() const { return m_a; }

  Point(unsigned int s, double a) : m_s(s), m_a(a), m_s2(1.0 * s * s), m_sd(a * s * s), m_sum_s2{}, m_sum_sd{}, m_ols_a{} { }
  Point() = default;
};

#ifdef CWDEBUG
std::ostream& operator<<(std::ostream& os, Point const& p)
{
  os << "{s:" << p.loop_size() << ", a:" << p.slope() << ", 1/a:" << (1 / p.slope()) << ", d:" << p.delay() << ", s2:" << p.m_s2 << ", sd:" << p.m_sd << "}";
  return os;
}
#endif

unsigned int DelayLoopCalibrationBase::peak_detect(double goal
    COMMA_CWDEBUG_ONLY(std::string)     // Parameter only exists when compiling with CWDEBUG,
#ifdef GNUPLOT
                           title        // but not used unless GNUPLOT is defined.
#endif
    )
{
  DoutEntering(dc::delayloop, "DelayLoopCalibrationBase::peak_detect(" << goal << ")");
#ifdef GNUPLOT
  // For debug purposes; plot graphs.
  bool draw_graph = !title.empty();
  eda::Plot plot1(title, "Loop size", "Slope");
  eda::Plot plot2(title, "Loop size", "Delay");
#endif
#ifdef INPUT_FILE
  // Do not do actual measurements but use pre-recorded data.
  std::ifstream infile;
  infile.open(INPUT_FILE);
#elif defined(OUTPUT_FILE)
  // Wrtie measurements to a file for later use with INPUT_FILE.
  std::ofstream outfile;
  outfile.open(OUTPUT_FILE);
#endif

  //===========================================================================
  // STEP 1
  //
  // Estimate a rough value for the needed loop size, where we assume that
  //
  //     delay(loop_size) = slope * loop_size.
  //
  double loop_size_estimate;
  {
    // Using https://en.wikipedia.org/wiki/Simple_linear_regression#Simple_linear_regression_without_the_intercept_term_(single_regressor)
    // we need to keep track of the sum of sd = loop_size * delay and the sum of s2 = loop_size * loop_size.
    double sum_sd = 0, sum_s2 = 0;
    double slope_estimate;                // The fitted slope of all measurements so far.

    // Approach a loop size (s) that results in goal milliseconds of delay.
    unsigned int s = 1000 * goal;         // Start very small since we don't know how slow the current hardware is and we don't want this calibration to take long.
    ASSERT(s > 0);
    do
    {
#ifdef INPUT_FILE
      double a;                           // The 'slope' of this single measurement.
      int z;
      infile >> s >> a >> z;
      double delay = a * s;
#else
      double delay = measure(s);
#ifdef OUTPUT_FILE
      double a = delay / s;
      outfile << s << ' ' << a << " 0\n";
#endif
#endif
      sum_sd += s * delay;
      sum_s2 += 1.0 * s * s;
      slope_estimate = sum_sd / sum_s2;
      s *= 2;                                   // Double the loop size every iteration.
    }
    while (s * slope_estimate < 1.41 * goal);   // Stop once goal has been more or less reached.
    // Calculate estimated loop size.
    loop_size_estimate = goal / slope_estimate;
  }

  //===========================================================================
  // STEP 2
  //
  // Collect number_of_measurements measurements using loop sizes ranging from
  // half loop_size_estimate till loop_size_estimate.
  //
  constexpr int number_of_measurements = 128;
  std::vector<Point> data;
  {
    double const sg64 = loop_size_estimate / number_of_measurements;

    // Do the measurements in batches of 32 linearly increasing loop size (in an attempt to decorrelate measurement with the same loop size).
    constexpr int number_of_iterations = number_of_measurements / 32;
    for (int iteration = 0; iteration < number_of_iterations; ++iteration) // Note: this might not be needed.
    {
      // Note: spreading the loop size over this range might not be needed (but does lower the total time needed to do the calibration).
      for (int step = number_of_measurements / 2; step <= number_of_measurements; ++step)
      {
        unsigned int s = std::round(step * sg64);       // The loop size to test.

        // Force a context switch in an attempt to decorrelate anomalies in the measured delay between measurements.
        sched_yield();          // Note: this might not be needed.
        // Do the actual measurement (or reading from input file / writing to output file).
#ifdef INPUT_FILE
        double a;
        int z;
        infile >> s >> a >> z;
        data.emplace_back(s, a);
#else
        double delay = measure(s);
        // Completely disregard measurements with absurd delays (note: this might not be needed).
        if (delay > 2 * goal)
        {
          --step;
          continue;
        }
        double a = delay / s;
        data.emplace_back(s, a);
#ifdef OUTPUT_FILE
        outfile << s << ' ' << a << " 0\n";
#endif
#endif
      }
    }
  }

#ifdef INPUT_FILE
  infile.close();
#elif defined(OUTPUT_FILE)
  outfile.close();
#endif

  //===========================================================================
  // STEP 3
  //
  // Estimate a loop size window in which 'fraction' of the measurements can be found.
  // We assume that the peak that we look for is the first peak in the "probability
  // density function" and that fraction is large enough to contain it. Hence,
  // the first window_end is past the start of this peak. We might find the same
  // fraction in a smaller window in the noise behond the peak, but that window
  // should then no longer include the peak itself anymore (because the density
  // goes down at first, after the peak):
  //
  // For example,
  // in the graph below the measurement with the lowest value of the slope is 'a'.
  // The first (and highest) peak is at b.
  //
  // [ Note: we can't use the fact that in THIS histogram the peak is the highest,
  // because we don't know what bucket size to use. A bucket size that is too small
  // would lead to a count of 1 in each bucket, so there isn't a 'peak' at all.
  // And a bucket size that is too wide gives not enough precision where the peak
  // starts. Therefore the method below doesn't use a bucket size at all, but one
  // is used for this histogram to get something that roughly looks like the
  // expected probability density function (pdf). ]
  //
  // If we'd look for a window size that contains (at least) 7 measurements,
  // the first window would span [a, b], it includes the peak. The window
  // will get more narrow until we reach [b, d]. Because after that the peak
  // at b goes rapidly down, the window MUST grow in length to keep containing
  // the same number of measurements until find a window entirely in the second,
  // lower but wider "peak": [d, f]. The latter is then rejected because d is
  // larger than the end of the first window and hence can not contain the first
  // peak anymore.
  //
  //    ^
  // pdf|
  //    |       |
  //    |       |   |
  //    |       || |||
  //    | |  || || ||||
  //    -------------------> slope
  //      a     bc def
  //
  double window;
  unsigned int min_window_start = 0;
  constexpr double fraction = 0.1;
  {
    // The fraction is determined as follows. It should lead to a narrow window (the density around
    // the first peak is high, so that would result in a good detection). But not so narrow that
    // the number of measurements in the window drops so low that we run the risk of not having
    // the peak in the first window. At 64 measurements it is like never going to happen that
    // there are 3 measurements below the peak, so using a fraction of 4/64 = 0.0625 should do
    // the trick. However, the window should also not be so narrow that not a substatious portion
    // of the first peak fits into because in the case of noise it might start to look like the
    // second peak (rizing in height at first, before going down), in that case we need to include
    // the highest point so that we're sure that the density will go down.
    // I think that a fraction of 10% is a good value to use.
    unsigned int const measurements_in_window = std::round(fraction * data.size());
    size_t const first_window_end = measurements_in_window - 1;

    // Sort all data points by loop size.
    std::sort(data.begin(), data.end(), [](Point const& p1, Point const& p2){ return p1.slope() < p2.slope(); });
    window = 1.0;       // Something very large.
    for (size_t window_start = 0; window_start <= first_window_end && window_start + measurements_in_window - 1 < data.size(); ++window_start)
    {
      size_t const window_end = window_start + measurements_in_window - 1;
      double window_size = data[window_end].slope() - data[window_start].slope();
      if (window_size < window)
      {
        window = window_size;
        min_window_start = window_start;
        Dout(dc::delayloop, "Window start: " << min_window_start << " (" <<
            (1e8 * data[window_start].slope()) << " - " << (1e8 * data[window_end].slope()) << ") with width " << (1e8 * window_size));
      }
    }
  }

  loop_size_estimate = goal / data[min_window_start].slope();
#ifdef CWDEBUG
  unsigned int slope_e8 = 1e8 * data[min_window_start].slope();
#endif
  Dout(dc::delayloop, "Peak detection: slope estimate: " << slope_e8 << ", loop size estimate: " << loop_size_estimate);
  Dout(dc::delayloop, "Window size: " << window);

#ifdef GNUPLOT
  // For debugging purposes, make a plot with the frequency count per bucket.
  eda::PlotHistogram plot3("slope distribution", "slope * 1e8", "count", 1e8 * window);
#endif
#if defined (GNUPLOT) || (defined(OUTPUT_FILE) && !defined(INPUT_FILE))
  {
#ifdef GNUPLOT
    plot3.set_xrange(1044, 1800);
    plot3.add_data_point(slope_e8, 1, "cal");
#endif
    double const bucket_width = window;
    std::map<int, int> fca;             // Frequency Count of Slopes per bucket.
    for (auto&& p : data)
      fca[p.slope() / bucket_width]++;
#ifdef GNUPLOT
    for (auto&& dp : fca)
      plot3.add_data_point(dp.first * 1e8 * window, dp.second, "pdf");
#endif
#if defined(OUTPUT_FILE) && !defined(INPUT_FILE)
    std::ofstream pdf;
    pdf.open("pdf.txt");
    for (auto&& dp : fca)
      pdf << dp.first << ' ' << dp.second << '\n';
    pdf.close();
#endif
  }
#endif // defined (GNUPLOT) || (defined(OUTPUT_FILE) && !defined(INPUT_FILE))

  //===========================================================================
  // STEP 4
  //
  // We now have a bucket size that contains fraction of the total number of
  // measurements in (at least) one point. In the case of noise this might
  // not be the lowest value of loop_size that isn't noise, but rather the
  // left side of the distribution tail of this peak.
  //
  // In order to take that into account, now do a frequency count of the
  // number of measurements in each such bucket and then return the bucket
  // that has at least a count of 1 and together with a 'neighbors' right
  // neighbor buckets a count of at least 2/3 of the fraction. The latter is
  // necessary because there is small peak about 0.929 of the slope that
  // we want (which is usually MUCH higher) that MIGHT still contain a
  // count of 1/4 of the fraction or so in exceptional cases.
  //
  unsigned int loop_size = loop_size_estimate;
  {
    constexpr int measurements_in_window = number_of_measurements * fraction * 0.666667;  // 2/3 of the fraction.
    constexpr int neighbors = 8;
    double max_window_size = neighbors * window;
    for (size_t window_start = 0; window_start + measurements_in_window - 1 < data.size(); ++window_start)
    {
      size_t const window_end = window_start + measurements_in_window - 1;
      double window_size = data[window_end].slope() - data[window_start].slope();
      Dout(dc::delayloop, "Window with start at " << window_start << " (" << (1e8 * data[window_start].slope()) <<
          " containing " << measurements_in_window << " measurements is " << (window_size / window) << " bucket widths wide.");
      if (window_size <= max_window_size)
      {
        loop_size = goal / data[window_start].slope();
        Debug(slope_e8 = 1e8 * data[window_start].slope());
        break;
      }
    }
  }

  Dout(dc::delayloop, "Final slope: " << slope_e8 << ", final loop size: " << loop_size);

#ifdef GNUPLOT
  constexpr double bottom_slope = 1.044e-5; //1.091e-5;
  constexpr double peak_slope = 1.124e-5; //1.175e-5;

  if (draw_graph)
  {
    for (size_t i = 0; i < data.size(); ++i)
    {
      plot1.add_data_point(data[i].loop_size(), data[i].slope(), 0, "data");
      plot2.add_data_point(data[i].loop_size(), data[i].delay(), 0, "data");
    }

    plot2.add("set key top left");
    plot1.add("set key font \",6\"");
    plot2.add("set key font \",6\"");

    std::stringstream ss2;
    ss2 << "bottom(x) = " << bottom_slope;
    plot1.add(ss2.str().c_str());
    ss2 << "*x";
    plot2.add(ss2.str().c_str());

    ss2.str(std::string());
    ss2 << "peak(x) = " << peak_slope;
    plot1.add(ss2.str().c_str());
    ss2 << "*x";
    plot2.add(ss2.str().c_str());

    ss2.str(std::string());
    ss2 << "fit(x) = " << (1e-8 * slope_e8);
    plot1.add(ss2.str().c_str());
    ss2 << "*x";
    plot2.add(ss2.str().c_str());

    plot1.function("bottom(x)");
    plot2.function("bottom(x)");
    plot1.function("peak(x)");
    plot2.function("peak(x)");
    plot1.function("fit(x)");
    plot2.function("fit(x)");

    ss2.str(std::string());
    ss2 << "set arrow from " << slope_e8 << ",graph(0,0) to " << slope_e8 << ",graph(1,1) nohead lc rgb \'red\'";
    plot3.add(ss2.str().c_str());

    if (static_cast<int>(slope_e8) != 1125 && static_cast<int>(slope_e8) != 1175 && loop_size != data[0].loop_size())
    {
      plot3.show();
      plot2.show();
      plot1.show();
    }
  }
#endif

  return loop_size;
}

} // namespace utils
