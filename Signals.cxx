// SPDX-FileCopyrightText: 2019, 2021-2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class Signals.
 */

#include "sys.h"
#include "debug.h"
#include "Signals.h"
#include "macros.h"
#include <string>
#include <cstring>
#include <cerrno>
#ifdef CWDEBUG
#include <libcwd/threads_created.h>
#endif

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
Channel signals("SIGNALS");
NAMESPACE_DEBUG_CHANNELS_END
#endif

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

#ifdef CWDEBUG
std::string signum_to_str(int signum)
{
  return 0 < signum && signum <= max_signum ? signal_name_str(signum) : std::to_string(signum);
}
#endif

Signal::~Signal()
{
}

SignalSet::SignalSet(SignalSet::Blocked)
{
  // This should never happen, but rather than throw lets just set all bits to zero (aka, "nothing is blocked").
  int ret = ::pthread_sigmask(SIG_BLOCK, nullptr, &set_);
  if (AI_UNLIKELY(ret != 0))
  {
    Dout(dc::warning, "::pthread_sigmask(SIG_BLOCK, nullptr, &set_) failed: " << std::strerror(ret));
    ::sigemptyset(&set_);
  }
}

void SignalSet::print_on(std::ostream& os) const
{
  char const* separator = "";
  for (int signum = 1; signum <= max_signum; ++signum)
    if (ismember(signum))
    {
      os << separator << signal_name_str(signum);
      separator = ", ";
    }
}

void Signal::print_on(std::ostream& os) const
{
  os << "Reserved signals: " << m_reserved_signals << "; reserved number of RT signals: " << m_number_of_RT_signals << '.';
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
  ASSERT(!libcwd::init_functions::threads_created);
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
  DoutEntering(dc::signals, "Signal::priv_next_rt_signum() with m_next_rt_signum = " << m_next_rt_signum << "; m_number_of_RT_signals = " << m_number_of_RT_signals << ".");
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
  DoutEntering(dc::signals, "Signal::reserve({" << signum_names << "}, " << number_of_RT_signals << ")");
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
    if (!m_reserved_signals.ismember(signum))
    {
      if (sigaction(signum, &action, NULL) == -1)
        DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
      m_reserved_signals.addset(signum);
    }
    else
      Dout(dc::warning, signal_name_str(signum) << " was already registered.");
  }
  // Block all reserved signals before creating any threads.
  SignalSet rt_signals;
  for (unsigned int signum = SIGRTMIN; signum < SIGRTMIN + number_of_RT_signals; ++signum)
    rt_signals.addset(signum);
  SignalSet all_signals = m_reserved_signals | rt_signals;
  all_signals.block();
}

void Signal::register_callback(int signum, void (*cb)(int))
{
  DoutEntering(dc::signals, "Signal::register_callback(" << signum_to_str(signum) << ", " << (void*)cb << ")");

  if (0 < signum && signum <= max_signum)
  {
    // No locking necessary because m_reserved_signals is initialized before any threads are created.
    if (!m_reserved_signals.ismember(signum))
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
  ASSERT(!m_callback_set.ismember(signum));
  m_callback_set.addset(signum);
#endif
}

//static
void Signal::unblock(int signum, void (*cb)(int))
{
  DoutEntering(dc::signals, "Signal::unblock(" << signum_to_str(signum) << ", " << (void*)cb << ")");

  SignalSet sigmask;
  sigmask.addset(signum);
  if (cb != SIG_IGN)
    instance().register_callback(signum, cb);
  sigmask.unblock();
}

// [[deprecated]] static
void Signal::unblock(sigset_t* sigmask, int signum, void (*cb)(int))
{
  // Use this instead (if you even use `sigmask`):
  sigemptyset(sigmask);
  sigaddset(sigmask, signum);
  unblock(signum, cb);
}

//static
void Signal::block_and_unregister(int signum)
{
  DoutEntering(dc::signals, "Signal::block_and_unregister(" << signum_to_str(signum) << ")");

  // Block signum.
  SignalSet sigmask;
  sigmask.addset(signum);
  sigmask.block();

  // Unregister signum.
  std::lock_guard<std::mutex> lock(instance().m_callback_mutex);
  struct sigaction action;
  std::memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = SIG_IGN;
  if (sigaction(signum, &action, NULL) == -1)
    DoutFatal(dc::core|error_cf, "sigaction(" << signum << ", " << (void*)&action << ", NULL) = -1");
  instance().m_reserved_signals.delset(signum);
#if CW_DEBUG
  instance().m_callback_set.delset(signum);
#endif
}

Signal::BlockGuard::BlockGuard(std::initializer_list<int> signums)
{
#ifdef CWDEBUG
  std::string signum_names;
  char const* separator = "";
  for (int signum : signums)
  {
    signum_names += separator + signal_name_str(signum);
    separator = ", ";
  }
  DoutEntering(dc::signals, "Signal::BlockGuard::BlockGuard({" << signum_names << "})");
#endif
  SignalSet mask;
  for (int signum : signums)
    mask.addset(signum);
  active_ = mask.block(previous_);
  Dout(dc::signals, "Blocked signals are now: " << utils::SignalSet{utils::SignalSet::blocked_signals});
}

Signal::BlockGuard::~BlockGuard()
{
  DoutEntering(dc::signals, "Signal::BlockGuard::~BlockGuard()");
  if (active_)
    previous_.setmask();
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
