// SPDX-FileCopyrightText: 2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class DelayLoopCalibration.
 */

#include "debug.h"
#include "utils/Global.h"
#include <chrono>

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
extern Channel delayloop;
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {

// Base class for DelayLoopCalibration.
class DelayLoopCalibrationBase
{
 public:
  enum N_Instance { total_required_measurements };
  class TotalRequiredMeasurements
  {
    unsigned int const m_n;
    unsigned int total_required_measurements() const;
   public:
    TotalRequiredMeasurements() : m_n(total_required_measurements()) { }
    operator unsigned int() const { return m_n; }
  };
  using GlobalTotalRequiredMeasurements = Global<TotalRequiredMeasurements, total_required_measurements, GlobalConverterVoid>;
  using clock_type = std::chrono::steady_clock;
  static constexpr double p = 0.99;             // Assumed (independent) chance to measure a non-outlier (as a result of an interrupt).
  static constexpr unsigned int m = 20;         // The number of (lowest) non-outliers to average over.
  static constexpr double epsilon = 1e-12;      // The maximum acceptable chance that we include an outlier in such average.

  // Do a single measurement.
  virtual double measure(unsigned int s) = 0;

  // Do n measurements with parameter s and return the average of the lowest m.
  double avg_of(unsigned int s);

  // Fit M with required accuracy around goal.
  unsigned int peak_detect(double goal COMMA_CWDEBUG_ONLY(std::string title = std::string()));

  // Do a sort of binary search to zone in on goal, start with hint.
  unsigned int search_lowest_of(unsigned int nm, double goal, unsigned int hint = 1);

 protected:
  virtual void set_fit_params(double a, double b) = 0;
};

// class DelayLoopCalibration
//
// Finds a linear approximation for the delay in milliseconds (as double)
// as function of a (loop size) parameter s.
//
//   delay = m_a * s + m_b
//
// Usage:
//
//   // The delay loop that has to be calibrated.
//   auto delay_loop = [](unsigned int s){
//     for (unsigned int i = 0; i < s; ++i)       // This code is just an example.
//       cpu_relax();
//   };
//
//   // This requires C++17.
//   DelayLoopCalibration delay_loop_calibration(delay_loop);
//
//   double const goal = 1.0;     // The required delay time in milliseconds.
//   unsigned int best_s = delay_loop_calibration.run(goal);
//
// Expected accuracy ~ +/- 5%, but in rare cases might cause the delay
// loop to be up to twice as slow as goal.
//
// One can also retrieve an properly averaged delay time of the delay loop
// for a given value of s with
//
//   double delay = delay_loop_calibration.avg_of(s);
//
template<typename M>
class DelayLoopCalibration : public DelayLoopCalibrationBase
{
 private:
  M m_measure;          // The function to fit.
  double m_a;           // The fitted slope of the function.
  double m_b;           // The fitted offset of the function.

 public:
  DelayLoopCalibration(M measure) : m_measure(measure) { }

  double __attribute__ ((noinline)) measure(unsigned int s) override
  {
    auto start = clock_type::now();
    m_measure(s);
    auto stop = clock_type::now();
    return std::chrono::duration<double, std::milli>(stop - start).count();
  }

  double a() const { return m_a; }
  double b() const { return m_b; }

 protected:
  void set_fit_params(double a, double b) override
  {
    m_a = a;
    m_b = b;
  }
};

} // namespace utils
