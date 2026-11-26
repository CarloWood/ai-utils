// SPDX-FileCopyrightText: 2019, 2021-2023, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of SignalSet.
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   2026/11/26
 *   - Moved SignalSet from Signals.h to its own header and added the initializer_list constructor.
 */

#pragma once

#include "utils/has_print_on.h"
#include <initializer_list>
#include <iosfwd>
#include <signal.h>             // Needed for POSIX declarations like pthread_sigmask, sigset_t, etc.

namespace utils {

// This class defines a print_on method.
using has_print_on::operator<<;

// A set of signal numbers, stored as the sigset_t that the system calls expect.
//
// This is the type with which to pass a set of signals around, for example to block them
// or to wait for them (see utils::threading::EventLoop::watch_signals).
// Every valid signal number fits, including the real-time signals up to SIGRTMAX.
// The constructor from Blocked and print_on are defined in Signals.cxx.
struct SignalSet
{
 private:
  sigset_t set_;

  // Create an uninitialized SignalSet.
  struct Uninitialized {};
  SignalSet(Uninitialized) { }

 public:
  SignalSet() { ::sigemptyset(&set_); }
  SignalSet(sigset_t const* set) : set_(*set) { }
  // Create the set of signums, for example SignalSet{SIGINT, SIGTERM}. Invalid signal numbers are not added.
  SignalSet(std::initializer_list<int> signums)
  {
    ::sigemptyset(&set_);
    for (int signum : signums)
      ::sigaddset(&set_, signum);
  }

  void emptyset() { ::sigemptyset(&set_); }
  bool ismember(int signum) const { return ::sigismember(&set_, signum) == 1; }
  void addset(int signum) { ::sigaddset(&set_, signum); }
  void delset(int signum) { ::sigdelset(&set_, signum); }

  // Return true if no signal is a member.
  bool empty() const
  {
#ifdef _GNU_SOURCE
    return ::sigisemptyset(&set_) == 1;
#else
    for (int signum = 1; signum < NSIG; ++signum)
      if (ismember(signum))
        return false;
    return true;
#endif
  }

  // Access the underlying sigset_t, for system calls that take one (e.g. signalfd).
  sigset_t const& sigset() const { return set_; }

  bool block() const { return ::pthread_sigmask(SIG_BLOCK, &set_, nullptr) == 0; }
  bool block(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_BLOCK, &set_, &previous_out.set_) == 0; }
  bool unblock() const { return ::pthread_sigmask(SIG_UNBLOCK, &set_, nullptr) == 0; }
  bool unblock(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_UNBLOCK, &set_, &previous_out.set_) == 0; }
  bool setmask() const { return ::pthread_sigmask(SIG_SETMASK, &set_, nullptr) == 0; }
  bool setmask(SignalSet& previous_out) const { return ::pthread_sigmask(SIG_SETMASK, &set_, &previous_out.set_) == 0; }

  // Read the currently blocked signal mask into this object.
  // This is mostly intended for debugging purposes; normally you should already know what signals you blocked.
  struct Blocked {};
  static constexpr Blocked blocked_signals{};
  SignalSet(Blocked);

  friend SignalSet operator|(SignalSet lhs, SignalSet rhs)
  {
    SignalSet result(Uninitialized{});
    sigorset(&result.set_, &lhs.set_, &rhs.set_);
    return result;
  }

  void print_on(std::ostream& os) const;
};

} // namespace utils
