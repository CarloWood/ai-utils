// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Declaration of AISignals.
//
// Copyright (C) 2019  Carlo Wood.
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
//
// CHANGELOG
//   and additional copyright holders.
//
//   2019/06/09
//   - Initial version, written by Carlo Wood.

#pragma once

#include "utils/Singleton.h"
#include <vector>
#include <csignal>
#include <iosfwd>
#include <mutex>

namespace utils {

class Signals : public Singleton<Signals>
{
  friend_Instance;
 private:
  Signals() : m_number_of_RT_signals(0), m_next_rt_signum(SIGRTMIN) { sigemptyset(&m_reserved_signals); }
  ~Signals();
  Signals(Signals const&) = delete;

  sigset_t m_reserved_signals;          // The signals that this application is interested in. All other signals use their default handler.
  std::mutex m_next_rt_signum_mutex;
  int m_number_of_RT_signals;
  int m_next_rt_signum;
  std::mutex m_callback_mutex;
#if CW_DEBUG
  sigset_t m_callback_set;
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
  static void unblock(sigset_t* sigmask, int signum, void (*cb)(int));
  static void unblock(sigset_t const* sigmask) { sigprocmask(SIG_UNBLOCK, sigmask, NULL); }
  static void unblock(int signum, void (*cb)(int) = SIG_IGN) { sigset_t sigmask; unblock(&sigmask, signum, cb); }
  static void default_handler(int signum) { unblock(signum, SIG_DFL); }
  static void block_and_unregister(int signum);

  void print_on(std::ostream& os) const;
};

} // namespace utils

// Usage, in main(), before creating any thread use:
//
//      AISignals signals({SIGINT, SIGABRT}, 3);     // Use signals SIGINT, SIGABRT and three RT signals.
//
// Obtain the three reserved RT signal numbers by three calls to utils::Signals::next_rt_signum().
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
//      utils::Signals::unblock(signum);
//
// It is also possible to combine this with setting a handler:
//
//      utils::Signals::unblock(signum, my_callback);
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

class AISignals
{
 public:
  AISignals() { }

  AISignals(int signum, unsigned int number_of_RT_signals = 0);
  AISignals(std::vector<int> signums, unsigned int number_of_RT_signals = 0);
  AISignals(std::initializer_list<int> signums, unsigned int number_of_RT_signals = 0) { AISignals(std::vector<int>(signums), number_of_RT_signals); }

  void register_callback(int signum, void (*cb)(int));
  void default_handler(int signum);

  friend std::ostream& operator<<(std::ostream& os, AISignals const& signals);
};
