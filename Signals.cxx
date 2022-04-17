/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class Signals.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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
#include "debug.h"
#include "Signals.h"
#include "macros.h"
#include <string>
#include <cstring>

namespace utils {

namespace {
SingletonInstance<Signal> dummy __attribute__ ((unused));

int constexpr max_signum = 31;

std::string signal_name_str(int signum)
{
  // Only pass real signals to Signals, that are defined in <csignal>.
  // Do not pass Real Time signal numbers [SIGRTMIN, SIGRTMAX]; these can't be hardcoded.
  // Instead, request the number you need as second parameter to Signals and use Signal::instance().next_rt_signum().
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

Signal::~Signal()
{
}

void Signal::print_on(std::ostream& os) const
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

void Signal::priv_reserve(int number_of_RT_signals)
{
  int prev_number_of_RT_signals = m_number_of_RT_signals;
  m_number_of_RT_signals += number_of_RT_signals;
  // Instantiate Signals object(s) before creating any threads!
  // This is necessary because threads created before instantiating Signals will receive all signals,
  // which is probably not what you want. Instead register the signals that your thread is interested
  // in afterwards, at the start of your thread.
#ifdef CWDEBUG
  ASSERT(!NAMESPACE_DEBUG::threads_created);
#endif
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

int Signal::priv_next_rt_signum()
{
  std::lock_guard<std::mutex> lock(m_next_rt_signum_mutex);
  DoutEntering(dc::notice, "Signal::priv_next_rt_signum() with m_next_rt_signum = " << m_next_rt_signum << "; m_number_of_RT_signals = " << m_number_of_RT_signals << ".");
  // Reserve the number of required Real Time signals in advance by creating an Signals object at the start of main().
  // In initializer lists of (global) objects, use utils::Signal::reserve_and_next_rt_signum().
  ASSERT(m_next_rt_signum < SIGRTMIN + m_number_of_RT_signals);
  return m_next_rt_signum++;
}

int Signal::priv_reserve_and_next_rt_signum()
{
  priv_reserve(1);
  return priv_next_rt_signum();
}

void Signal::reserve(std::vector<int> const& signums, unsigned int number_of_RT_signals)
{
#ifdef CWDEBUG
  std::string signum_names;
  char const* separator = "";
  for (int signum : signums)
  {
    signum_names += separator + signal_name_str(signum);
    separator = ", ";
  }
  DoutEntering(dc::notice, "Signal::reserve({" << signum_names << "}, " << number_of_RT_signals << ")");
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

void Signal::register_callback(int signum, void (*cb)(int))
{
  DoutEntering(dc::notice, "Signal::register_callback(" << signum << ", " << (void*)cb << ")");
  if (0 < signum && signum <= max_signum)
  {
    // No locking necessary because m_reserved_signals is initialized before any threads are created.
    if (sigismember(&m_reserved_signals, signum) != 1)
      // Register signals first by creating an Signals object at the start of main.
      // Note that you cannot specify a callback for SIGKILL or SIGSTOP.
      DoutFatal(dc::core, "Trying to register a callback for a signal (" << signal_name_str(signum) << ") that was not registered.");
  }
  else if (!(SIGRTMIN <= signum && signum < SIGRTMIN + m_next_rt_signum))
    // Only use RT signals that were returned by Signal::priv_next_rt_signum.
    DoutFatal(dc::core, "Trying to register a callback for RT signal " << (signum - SIGRTMIN) << " which is not a value returned by Signals::next_rt_signum().");

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
void Signal::unblock(sigset_t* sigmask, int signum, void (*cb)(int))
{
  DoutEntering(dc::notice, "Signal::unblock(sigmask, " << signum << ", " << (void*)cb << ")");
  sigemptyset(sigmask);
  sigaddset(sigmask, signum);
  if (cb != SIG_IGN)
    instance().register_callback(signum, cb);
  unblock(sigmask);
}

//static
void Signal::block_and_unregister(int signum)
{
  DoutEntering(dc::notice, "Signal::block_and_unregister(" << signum << ")");
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
  sigdelset(&instance().m_reserved_signals, signum);
#if CW_DEBUG
  sigdelset(&instance().m_callback_set, signum);
#endif
}

Signals::Signals(std::vector<int> signums, unsigned int number_of_RT_signals)
{
  utils::Signal::instance().reserve(signums, number_of_RT_signals);
}

Signals::Signals(int signum, unsigned int number_of_RT_signals)
{
  utils::Signal::instance().reserve({signum}, number_of_RT_signals);
}

void Signals::register_callback(int signum, void (*cb)(int))
{
  utils::Signal::instance().register_callback(signum, cb);
}

void Signals::default_handler(int signum)
{
  utils::Signal::instance().default_handler(signum);
}

std::ostream& operator<<(std::ostream& os, Signals const&)
{
  utils::Signal::instance().print_on(os);
  return os;
}

} // namespace utils
