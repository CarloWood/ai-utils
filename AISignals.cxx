// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class AISignal.
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
#include "debug.h"
#include "AISignals.h"
#include "macros.h"
#include <string>

namespace utils {

namespace {
SingletonInstance<Signals> dummy __attribute__ ((unused));

int constexpr max_signum = 31;

std::string signal_name_str(int signum)
{
  // Only pass real signals to AISignal, that are defined in <csignal>.
  // Do not pass Real Time signal numbers [SIGRTMIN, SIGRTMAX]; these can't be hardcoded.
  // Instead, request the number you need as second parameter to AISignals and use Signal::instance().next_rt_signum().
  ASSERT(0 < signum && signum <= max_signum);
  switch (signum)
  {
    AI_CASE_RETURN(SIGHUP);
    AI_CASE_RETURN(SIGINT);
    AI_CASE_RETURN(SIGQUIT);
    AI_CASE_RETURN(SIGILL);
    AI_CASE_RETURN(SIGTRAP);
    AI_CASE_RETURN(SIGABRT);
    AI_CASE_RETURN(SIGBUS);
    AI_CASE_RETURN(SIGFPE);
    AI_CASE_RETURN(SIGKILL);
    AI_CASE_RETURN(SIGUSR1);
    AI_CASE_RETURN(SIGSEGV);
    AI_CASE_RETURN(SIGUSR2);
    AI_CASE_RETURN(SIGPIPE);
    AI_CASE_RETURN(SIGALRM);
    AI_CASE_RETURN(SIGTERM);
    AI_CASE_RETURN(SIGSTKFLT);  // Obsolete.
    AI_CASE_RETURN(SIGCHLD);
    AI_CASE_RETURN(SIGCONT);
    AI_CASE_RETURN(SIGSTOP);
    AI_CASE_RETURN(SIGTSTP);
    AI_CASE_RETURN(SIGTTIN);
    AI_CASE_RETURN(SIGTTOU);
    AI_CASE_RETURN(SIGURG);
    AI_CASE_RETURN(SIGXCPU);
    AI_CASE_RETURN(SIGXFSZ);
    AI_CASE_RETURN(SIGVTALRM);
    AI_CASE_RETURN(SIGPROF);
    AI_CASE_RETURN(SIGWINCH);
    AI_CASE_RETURN(SIGPOLL);
    AI_CASE_RETURN(SIGPWR);
    AI_CASE_RETURN(SIGSYS);
  }
  return "Unknown signal";
}

} // namespace

Signals::~Signals()
{
}

void Signals::print_on(std::ostream& os) const
{
  os << "Reserved signals: ";
  char const* separator = "";
  for (int signum = 1; signum <= max_signum; ++signum)
    if (sigismember(&m_reserved_signals, signum) == 1)
    {
      os << separator << signal_name_str(signum);
      separator = ", ";
    }
  os << "; reserved number of RT signals: " << m_number_of_RT_signals << '.';
}

void Signals::priv_reserve(int number_of_RT_signals)
{
  int prev_number_of_RT_signals = m_number_of_RT_signals;
  m_number_of_RT_signals += number_of_RT_signals;
  // Instantiate AISignals object(s) before creating any threads!
  // This is necessary because threads created before instantiating AISignals will receive all signals,
  // which is probably not what you want. Instead register the signals that your thread is interested
  // in afterwards, at the start of your thread.
  ASSERT(!NAMESPACE_DEBUG::threads_created);
  // There are only 30 or so Real Time signals available.
  ASSERT(SIGRTMIN + m_number_of_RT_signals < SIGRTMAX);
  struct sigaction action;
  std::memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN;
  for (int i = 0; i < number_of_RT_signals; ++i)
  {
    int signum = SIGRTMIN + prev_number_of_RT_signals + i;
    if (sigaction(signum, &action, NULL) == -1)
      DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
  }
}

int Signals::priv_next_rt_signum()
{
  std::lock_guard<std::mutex> lock(m_next_rt_signum_mutex);
  DoutEntering(dc::notice, "Signals::priv_next_rt_signum() with m_next_rt_signum = " << m_next_rt_signum << "; m_number_of_RT_signals = " << m_number_of_RT_signals << ".");
  // Reserve the number of required Real Time signals in advance by creating an AISignals object at the start of main().
  // In initializer lists of (global) objects, use utils::Signals::reserve_and_next_rt_signum().
  ASSERT(m_next_rt_signum < SIGRTMIN + m_number_of_RT_signals);
  return m_next_rt_signum++;
}

int Signals::priv_reserve_and_next_rt_signum()
{
  priv_reserve(1);
  return priv_next_rt_signum();
}

void Signals::reserve(std::vector<int> const& signums, unsigned int number_of_RT_signals)
{
#ifdef CWDEBUG
  std::string signum_names;
  char const* separator = "";
  for (int signum : signums)
  {
    signum_names += separator + signal_name_str(signum);
    separator = ", ";
  }
  DoutEntering(dc::notice, "Signals::reserve({" << signum_names << "}, " << number_of_RT_signals << ")");
#endif
  priv_reserve(number_of_RT_signals);
  // Ignore all reserved signals (except SIGKILL and SIGSTOP).
  struct sigaction action;
  std::memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN;
  for (int signum : signums)
  {
    if (signum == SIGKILL || signum == SIGSTOP)
      continue;
    if (sigismember(&m_reserved_signals, signum) != 1)
    {
      if (sigaction(signum, &action, NULL) == -1)
        DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
      sigaddset(&m_reserved_signals, signum);
    }
    else
      Dout(dc::warning, signal_name_str(signum) << " was already registered.");
  }
  // Block all reserved signals before creating any threads.
  sigset_t rt_signals;
  sigemptyset(&rt_signals);
  for (unsigned int signum = SIGRTMIN; signum < SIGRTMIN + number_of_RT_signals; ++signum)
    sigaddset(&rt_signals, signum);
  sigset_t all_signals;
  sigemptyset(&all_signals);
  sigorset(&all_signals, &m_reserved_signals, &rt_signals);
  sigprocmask(SIG_BLOCK, &all_signals, nullptr);
}

void Signals::register_callback(int signum, void (*cb)(int))
{
  DoutEntering(dc::notice, "Signals::register_callback(" << signum << ", " << (void*)cb << ")");
  if (0 < signum && signum <= max_signum)
  {
    // No locking necessary because m_reserved_signals is initialized before any threads are created.
    if (sigismember(&m_reserved_signals, signum) != 1)
      // Register signals first by creating an AISignal object at the start of main.
      // Note that you cannot specify a callback for SIGKILL or SIGSTOP.
      DoutFatal(dc::core, "Trying to register a callback for a signal (" << signal_name_str(signum) << ") that was not registered.");
  }
  else if (!(SIGRTMIN <= signum && signum < SIGRTMIN + m_next_rt_signum))
    // Only use RT signals that were returned by Signals::priv_next_rt_signum.
    DoutFatal(dc::core, "Trying to register a callback for RT signal " << (signum - SIGRTMIN) << " which is not a value returned by AISignals::next_rt_signum().");

  std::lock_guard<std::mutex> lock(m_callback_mutex);
  struct sigaction action;
  std::memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = cb;
  if (sigaction(signum, &action, NULL) == -1)
    DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
#if CW_DEBUG
  // Please only register a callback once for any given signal number. This assert tries to avoid two different threads trying to register the same signal number.
  ASSERT(sigismember(&m_callback_set, signum) != 1);
  sigaddset(&m_callback_set, signum);
#endif
}

//static
void Signals::unblock(sigset_t* sigmask, int signum, void (*cb)(int))
{
  DoutEntering(dc::notice, "Signals::unblock(sigmask, " << signum << ", " << (void*)cb << ")");
  sigemptyset(sigmask);
  sigaddset(sigmask, signum);
  if (cb != SIG_IGN)
    instance().register_callback(signum, cb);
  unblock(sigmask);
}

//static
void Signals::block_and_unregister(int signum)
{
  DoutEntering(dc::notice, "Signals::block_and_unregister(" << signum << ")");
  sigset_t sigmask;
  sigemptyset(&sigmask);
  sigaddset(&sigmask, signum);
  sigprocmask(SIG_BLOCK, &sigmask, NULL);
  std::lock_guard<std::mutex> lock(instance().m_callback_mutex);
  struct sigaction action;
  std::memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN;
  if (sigaction(signum, &action, NULL) == -1)
    DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
#if CW_DEBUG
  sigdelset(&instance().m_callback_set, signum);
#endif
}

} // namespace utils

AISignals::AISignals(std::vector<int> signums, unsigned int number_of_RT_signals)
{
  utils::Signals::instance().reserve(signums, number_of_RT_signals);
}

AISignals::AISignals(int signum, unsigned int number_of_RT_signals)
{
  utils::Signals::instance().reserve({signum}, number_of_RT_signals);
}

void AISignals::register_callback(int signum, void (*cb)(int))
{
  utils::Signals::instance().register_callback(signum, cb);
}

void AISignals::default_handler(int signum)
{
  utils::Signals::instance().default_handler(signum);
}

std::ostream& operator<<(std::ostream& os, AISignals const&)
{
  utils::Signals::instance().print_on(os);
  return os;
}
