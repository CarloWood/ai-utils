// Copyright (C) 2004, by
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file may be distributed under the terms of the Q Public License
// version 1.0 as appearing in the file LICENSE.QPL included in the
// packaging of this file.
//
#include "sys.h"
#include <iostream>

#include "debug.h"
#include "Singleton.h"

#undef ERROR1		// Forgetting the friend_Instance in singleton class.
#undef ERROR2		// Using Global instead of Singleton for singleton.
#undef ERROR2a		// Trying to use Global instead of Singleton in order to pass a parameter.
#undef ERROR3		// Instantiating the same Global twice.
#undef ERROR4		// Instantiating the same Singleton twice.   TEST IS BROKEN!?
#undef ERROR5		// Forgetting to instantiate a global Global<> object.
#undef ERROR5a		// Using `instance()' from global constructor.
#undef ERROR5b		// Not instantiating it at all and using `instance()' from main().
#undef ERROR6		// Forgetting to instantiate a global Singleton<> object.
#undef ERROR6a		// But with instantiation through `instantiate()'.
#undef ERROR7		// Forgetting to make constructor of singleton private.
#undef ERROR8		// Forgetting an `instantiate()' inside a global constructor.

#if (defined(ERROR5a) || defined(ERROR5b)) && !defined(ERROR5)
#define ERROR5
#endif
#if defined(ERROR6a) && !defined(ERROR6)
#define ERROR6
#endif
#if defined(ERROR2a) && !defined(ERROR2)
#define ERROR2
#endif

class Foo : public Singleton<Foo> {
  friend_Instance;
private:
  int data;

  Foo(void) : data(1) { }
  ~Foo() { }
  Foo(Foo const& foo) : Singleton<Foo, Foo>(foo) { }

public:
  int get_data(void) const { return data; }
};

class Test {
public:
  Test(int) { }
  Test(void) : j(0) { }
  int j;
};

class GlobalConverterTest2 {
public:
  const char *operator()(int) {
    return "Test2";
  }
};

class Test2 {
  const char *s;
public:
  Test2(const char *s_) : s(s_) { }
  Test2(void) : s("default constructor") { }
  char const* str(void) const { return s; }
};

class Instance { };	// This doesn't confuse the `friend' in the below class.

class Bar : public Singleton<Bar> {
#ifndef ERROR1
  friend_Instance;
#else
  friend class Test;
#endif
private:
  int data;

#ifdef ERROR7
public:
#endif
  Bar(void) : data(2) {
#ifdef ERROR5a
    Global<Test, 2>::instance().j = 9;
#else
#ifndef ERROR5b
    Global<Test, 2>::instantiate().j = 9;		// Ok
#endif
#endif
#ifdef ERROR8
    data = Singleton<Foo>::instance().get_data();
#endif
  }
#ifdef ERROR2a
  Bar(int) { }
#endif
  ~Bar() { }
private:
  Bar(Bar const& bar) : Singleton<Bar>(bar) { }

public:
  int get_data(void) const { return data; }
};

static SingletonInstance<Foo> singletonFoo __attribute__ ((unused));
static Global<Test, 0> globalTest0;
static Global<Test, 1> globalTest1;
#ifndef ERROR5
static Global<Test, 2> globalTest2;
#endif
#ifdef ERROR2
#ifdef ERROR2a
static Global<Bar, -1, GlobalConverterInt> globalBarError;
#else
static Global<Bar, 0, GlobalConverterVoid> globalBarError;
#endif
#else
#ifndef ERROR6
static SingletonInstance<Bar> singletonBar __attribute__ ((unused));
#endif
#endif
#ifdef ERROR3
static Global<Test, 0> test;
#endif
#ifdef ERROR4
static SingletonInstance<Foo> test __attribute__ ((unused));
#endif

namespace {
  static Global<Test2, 0, GlobalConverterTest2> S_dummy_Test2;
}

int main(int UNUSED_ARG(argc), char *UNUSED_ARG(argv)[])
{
  using std::cout;
  using std::endl;

#ifdef ERROR7
  Bar bar;
#endif
#ifdef ERROR6a
  Singleton<Bar>::instantiate();
#endif
#ifdef DEBUGGLOBAL
  GlobalObjectManager::main_entered();
#endif

  // Don't show allocations that are allocated before main()
  //make_all_allocations_invisible_except(NULL);

#ifdef CWDEBUG
  // Select debug channels
  ForAllDebugChannels( while (!debugChannel.is_on()) debugChannel.on() );
  DEBUGCHANNELS::dc::malloc.off();
#endif

  // Write debug output to cout
  Debug( libcw_do.set_ostream(&cout) );

  // Turn debug object on
  Debug( libcw_do.on() );

  // Print channels
  // Debug( list_channels_on(libcw_do) );

  cout << Singleton<Foo>::instance().get_data() << endl;
  cout << Singleton<Bar>::instance().get_data() << endl;
  cout << (void *)&Global<Test, 0>::instance() << endl;
  cout << (void *)&Global<Test, 2>::instance() << endl;
  cout << Global<Test2, 0>::instance().str() << endl;

  Debug( list_allocations_on(libcw_do) );

  return 0;
}
