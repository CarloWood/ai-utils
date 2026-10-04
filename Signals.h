// SPDX-FileCopyrightText: 2019, 2021-2023 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of Signals.
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   2019/06/09
 *   - Initial version, written by Carlo Wood.
 *
 *   2026/07/21
 *   - Changed license to MIT.
 */

#pragma once

#include "utils/Singleton.h"
#include "utils/has_print_on.h"
#include <vector>
#include <initializer_list>
#include <iosfwd>
#include <mutex>
#include <signal.h>             // Needed for POSIX declarations like pthread_sigmask, sigset_t, etc.
#include "debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
extern Channel signals;
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {
// This class defines a print_on method.
using has_print_on::operator<<;

struct SignalSet
{
 private:
  sigset_t set_;

 public:
  SignalSet() { ::sigemptyset(&set_); }
  SignalSet(sigset_t set) : set_(set) { }

  void emptyset() { ::sigemptyset(&set_); }
  bool ismember(int signum) const { return ::sigismember(&set_, signum) == 1; }
  void addset(int signum) { ::sigaddset(&set_, signum); }
  void delset(int signum) { ::sigdelset(&set_, signum); }

  bool block() const { return ::pthread_sigmask(SIG_BLOCK, &set_, nullptr) == 0; }
  bool block(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_BLOCK, &set_, &previous_out.set_) == 0; }
  bool unblock() const { return ::pthread_sigmask(SIG_UNBLOCK, &set_, nullptr) == 0; }
  bool unblock(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_UNBLOCK, &set_, &previous_out.set_) == 0; }
  bool setmask() const { return ::pthread_sigmask(SIG_SETMASK, &set_, nullptr) == 0; }
  bool setmask(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_SETMASK, &set_, &previous_out.set_) == 0; }

  friend SignalSet operator|(SignalSet lhs, SignalSet rhs)
  {
    sigset_t result;
    sigorset(&result, &lhs.set_, &rhs.set_);
    return {result};
  }

  void print_on(std::ostream& os) const;
};

class Signal : public Singleton<Signal>
{
  friend_Instance;
 private:
  Signal() : m_number_of_RT_signals(0), m_next_rt_signum(SIGRTMIN) { }
  ~Signal();
  Signal(Signal const&) = delete;

  SignalSet m_reserved_signals;         // The signals that this application is interested in. All other signals use their default handler.
  std::mutex m_next_rt_signum_mutex;
  int m_number_of_RT_signals;
  int m_next_rt_signum;
  std::mutex m_callback_mutex;
#if CW_DEBUG
  SignalSet m_callback_set;
#endif

 private:
  void priv_reserve(int number_of_RT_signals);
  int priv_reserve_and_next_rt_signum();
  int priv_next_rt_signum();

 public:
  void reserve(std::vector<int> const& signums, unsigned int number_of_RT_signals);

  void register_callback(int signum, void (*cb)(int));
  static int reserve_and_next_rt_signum() { return instantiate().priv_reserve_and_next_rt_signum(); }
  static int next_rt_signum() { return instance().priv_next_rt_signum(); }

  [[deprecated]] static void unblock(sigset_t* sigmask, int signum, void (*cb)(int));
  [[deprecated]] static void unblock(sigset_t const* sigmask) { ::pthread_sigmask(SIG_UNBLOCK, sigmask, NULL); }

  static void unblock(int signum, void (*cb)(int) = SIG_IGN);
  static void default_handler(int signum) { unblock(signum, SIG_DFL); }
  static void block_and_unregister(int signum);

  class BlockGuard final
  {
    friend class Signals;

   private:
    SignalSet previous_{};
    bool active_ = false;

   public:
    // Add `signums` to the set of blocked signals. Stores the previous signal mask in previous_.
    BlockGuard(std::initializer_list<int> signums);
    // Restore the signal mask to the value stored in previous_.
    ~BlockGuard();

    BlockGuard(BlockGuard const&) = delete;
    BlockGuard& operator=(BlockGuard const&) = delete;
    BlockGuard(BlockGuard&&) = delete;
    BlockGuard& operator=(BlockGuard&&) = delete;
  };

  void print_on(std::ostream& os) const;
};

// Usage, in main(), before creating any thread use:
//
//      Signals signals({SIGINT, SIGABRT}, 3);     // Use signals SIGINT, SIGABRT and three RT signals.
//
// Obtain the three reserved RT signal numbers by three calls to utils::Signal::next_rt_signum().
// See `man 7 signal` for a list of existing SIGnals.
//
// All these signals will be ignored by default (SIG_IGN). Signals that are not mentioned
// or reserved will keep their default handler (for example, by passing SIGINT that signal won't
// interrupt the program anymore (ie, pressing control-C will stop working), but if you don't
// pass it then that still works as normal).
//
// Signal handlers can be added for the signals that are passed (preferably from main) with
//
//      signals.register_callback(signum, my_callback);
//
// Or to reinstall the default handler:
//
//      signals.default_handler(signum);
//
// In threads, optionally unblock signals again with:
//
//      utils::Signal::unblock(signum);
//
// It is also possible to combine this with setting a handler:
//
//      utils::Signal::unblock(signum, my_callback);
//
// instead of using signals.register_callback.
//
// Note that a thread that is created inherits the signal mask of the parent thread;
// so when you unblock a signal and then create a thread, then that signal is also
// unblocked in the new thread.
//
// Signals are handled by a random thread of the threads that have that signal unblocked.
//
// Also note that handlers are per process. You can not set a different handler in
// different threads.

class Signals
{
 public:
  Signals() { }

  Signals(int signum, unsigned int number_of_RT_signals = 0);
  Signals(std::vector<int> signums, unsigned int number_of_RT_signals = 0);
  Signals(std::initializer_list<int> signums, unsigned int number_of_RT_signals = 0) : Signals(std::vector<int>(signums), number_of_RT_signals) { }

  void register_callback(int signum, void (*cb)(int));
  void default_handler(int signum);

  friend std::ostream& operator<<(std::ostream& os, Signals const& signals);
};

} // namespace utils
