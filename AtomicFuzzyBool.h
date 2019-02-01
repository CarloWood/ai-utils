#pragma once

#include "FuzzyBool.h"
#include <atomic>

namespace utils {

class AtomicFuzzyBool
{
 private:
   std::atomic_int m_aval;
   static_assert(std::atomic<FuzzyBoolEnum>::is_always_lock_free, "Get a real OS.");

 public:
  AtomicFuzzyBool() { }
  AtomicFuzzyBool(FuzzyBoolPOD const& val) : m_aval(val.m_val) { }

  void store(FuzzyBoolPOD const& desired, std::memory_order order = std::memory_order_seq_cst) noexcept { m_aval.store(desired.m_val, order); }
  FuzzyBool operator=(FuzzyBoolPOD const& val) { m_aval = val.m_val; return val; }

  FuzzyBool load(std::memory_order order = std::memory_order_seq_cst) const noexcept { return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.load(order))}; }
  explicit operator FuzzyBool() const { return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.load())}; }

  void print_on(std::ostream& os, std::memory_order order = std::memory_order_seq_cst) const { os << m_aval.load(order); }
  friend std::ostream& operator<<(std::ostream& os, AtomicFuzzyBool const& aval) { aval.print_on(os); return os; }

  friend FuzzyBool operator==(AtomicFuzzyBool const& aval, FuzzyBoolPOD const& val) { return aval.load() == val; }
  friend FuzzyBool operator==(FuzzyBoolPOD const& val, AtomicFuzzyBool const& aval) { return val == aval.load(); }
  friend FuzzyBool operator!=(AtomicFuzzyBool const& aval, FuzzyBoolPOD const& val) { return aval.load() != val; }
  friend FuzzyBool operator!=(FuzzyBoolPOD const& val, AtomicFuzzyBool const& aval) { return val != aval.load(); }

  bool always(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_true; }           // Returns true when True.
  bool likely(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) & fuzzy_was_true; }        // Returns true when WasTrue or True.
  bool unlikely(std::memory_order order = std::memory_order_seq_cst) const { return !(m_aval.load(order) & fuzzy_was_true); }   // Returns true when WasFalse or False.
  bool never(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_false; }           // Returns true when False.
#ifdef CWDEBUG
  bool has_same_value_as(FuzzyBool const& fb, std::memory_order order = std::memory_order_seq_cst) { return m_aval.load(order) == fb.m_val; }
#endif

  FuzzyBool fetch_invert(std::memory_order order = std::memory_order_seq_cst)
  {
    return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.fetch_xor(fuzzy_true, order))};  // Returns the old value.
  }

  FuzzyBool fetch_AND(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (AND_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_OR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (OR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_XOR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (XOR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_NOT_XOR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (NOT_XOR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }
};

} // namespace utils
