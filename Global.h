/**
 * \file Global.h
 * \brief Declaration of template class Global.
 *
 * Copyright (C) 2014 Aleric Inglewood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \class Global
 * \brief Initialization order fiasco free global instances.
 *
 * Usage:
 *
 * Note: Below we assume that you want to instantiate global objects of type `class Foo`.
 *
 * 1) Call GlobalObjectManager::main_entered() at the _very_ top of `main()`.
 *    This call signifies the end of the global constructors.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      int main(int argc, void* argv[])
 *      {
 *      #ifdef DEBUGGLOBAL
 *        GlobalObjectManager::main_entered();
 *      #endif
 *        //...
 *      }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * 2) Create a list of human readable integer constants, one for each
 *    instance that you want to instantiate in the current application.
 *    You may use any value except -1.  Other negative values are reserved.
 *
 *    For example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      enum FooInstance {
 *        red,
 *        green,
 *	  yellow
 *      };
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * 3) If you want to call the default constructor of `Foo`, use:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      typedef Global<Foo, red, GlobalConverterVoid> GlobalRedFoo;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    If you want the constructor `Foo(int instance)` to be called,
 *    where `instance` is the integer constant of that instance
 *    (`red` in the last example), then use:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      typedef Global<Foo, red> GlobalRedFoo;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    If you want other data to be passed to the constructor of
 *    `Foo`, then you should define a `GlobalConverter' class yourself.
 *    For example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      class GlobalConverterString {
 *      public:
 *        string operator()(int instance);
 *      };
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    Where `operator()` should convert the `instance` variable into
 *    a string.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      typedef Global<Foo, red, GlobalConverterString> GlobalRedFoo;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    This is especially useful for library classes since it allows
 *    the set of instances to be extended later, independ of the library.
 *
 * 4) For each instance of `Foo`, instantiate a Global<> object.
 *    For example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      namespace {
 *      GlobalRedFoo    redDummy;
 *      GlobalGreenFoo  greenDummy;
 *      GlobalYellowFoo yellowDummy;
 *      }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    Note that there is no need to use a typedef.
 *    You can do as well:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      namespace {
 *      Global<Foo, red,    GlobalConverterString> redDummy;
 *      Global<Foo, green>                         greenDummy;
 *      Global<Foo, yellow, GlobalConverterVoid>   yellowDummy;
 *      }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    The name of the dummy doesn't matter of course, as long as
 *    it doesn't collide - you don't even need the anonymous namespace actually:
 *    it's just handy to make sure that the dummy names won't collide.
 *
 * 5) Now access the instances as follows:
 *    In constructors of other Global<> objects and in constructors of real global/static objects, use:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      GlobalRedFoo::instantiate()
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    which returns a Foo& to the `red' instance.
 *
 *    For example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *      class Bar {
 *        int b;
 *      public:
 *        Bar(void) : b(Global<Color, blue>::instantiate().brightness()) { }
 *      };
 *
 *      Bar bar;	// Instantiate a real global object of class `Bar'.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *    Anywhere else (that is, in code called from `main()`) use `GlobalRedFoo::``#instance()`,
 *    or `Global<Foo, red>::``#instance()` because the converter class doesn't matter in this case.
 *
 *
 * If you want to check whether you did everything correctly, define `DEBUGGLOBAL`
 * and it will tell you exactly what you did wrong, if anything.
 */

#ifndef UTILS_GLOBAL_H
#define UTILS_GLOBAL_H

#ifndef USE_PCH
#include "debug.h"
#if defined(DEBUGGLOBAL) && (!defined(CWDEBUG) || !CWDEBUG_ALLOC)
#error "You cannot define DEBUGGLOBAL without defining CWDEBUG and CWDEBUG_ALLOC"
#endif
#ifdef DEBUGGLOBAL
#include <execinfo.h>
#endif
#include <new>
#endif

// Forward declarations.
template<class TYPE, int inst, class CONVERTER> class Global;
class GlobalObjectManager;
class GlobalConverterVoid;

/// @cond Doxygen_Suppress
// Private classes
namespace utils
{

namespace _internal_
{
  // *****************************************************
  // *                                                   *
  // *   NEVER USE ANYTHING FROM namespace _internal_ !  *
  // *                                                   *
  // *****************************************************

  // Base class for global objects
  class GlobalObject {
  friend class ::GlobalObjectManager;
  protected:
    virtual ~GlobalObject() { }
#ifdef DEBUGGLOBAL
    virtual bool instantiated_from_constructor(void) const = 0;
    virtual void print_type_name(std::ostream&) const = 0;
    virtual void set_initialized_and_after_global_constructors(void) const = 0;
#endif
  };

  template<class TYPE, int inst>
  class GlobalBase {
    // Needed to calculate the size of Global<>::Instance
    // and to calculate the offset between Instance* and TYPE*.
    class InstanceDummy : public TYPE, public GlobalObject {
      friend class GlobalObject; // To suppress a warning
      virtual ~InstanceDummy() throw() { }
    };
  protected:
    static char instance_[/*sizeof(InstanceDummy)*/];
    static char initialized;
#ifdef DEBUGGLOBAL
    static bool initialized_and_after_global_constructors;
    static bool instantiated_from_constructor;
    static char const* instantiate_function_name;
    static void const* instantiate_return_address1;
public:
    static void set_initialized_and_after_global_constructors(void) { initialized_and_after_global_constructors = initialized; }
    static void set_instantiate_return_address0(void const* addr)
	{ instantiate_function_name = libcwd::pc_mangled_function_name((char const*)addr + libcwd::builtin_return_address_offset); }
    static void set_instantiate_return_address1(void const* addr) { instantiate_return_address1 = addr; }
#endif

  public:
    static inline TYPE& instance(void);

#ifdef DEBUGGLOBAL
  private:
    static void print_error_msg(void);
    static void check_call_to_instance(void);

  public:
    static bool gifc_(void)
	{
	  return instantiated_from_constructor;
	}
    static bool beingInstantiatedRightNow_(void)
	{
	  return (initialized == -1);
	}
#endif
  };

  template<class TYPE, int inst, class CONVERTER>
  class Instance : public TYPE, public GlobalObject {
  private:
    static CONVERTER parameter_converter;

  private:	// Make sure nobody instantiates Instance itself except for Global<TYPE, inst, CONVERTER>.
    friend class Global<TYPE, inst, CONVERTER>;
    Instance(int) : TYPE(parameter_converter(inst)) { }	// TYPE is private (compile error)? Look at NOTE2 at the bottom of this file.
    virtual ~Instance() throw() { }

#ifdef DEBUGGLOBAL
    virtual bool instantiated_from_constructor(void) const;
    virtual void print_type_name(std::ostream&) const;
    virtual void set_initialized_and_after_global_constructors(void) const;
#endif
  };

  template<class TYPE, int inst>
  class Instance<TYPE, inst, GlobalConverterVoid> : public TYPE, public ::utils::_internal_::GlobalObject {
  private:	// Make sure nobody instantiates Instance itself except for Global<TYPE, inst, GlobalConverterVoid>.
    friend class Global<TYPE, inst, GlobalConverterVoid>;
    Instance(int) { }		// TYPE is private (compile error)? Look at NOTE1 at the bottom of this file.
    virtual ~Instance() throw() { }

#ifdef DEBUGGLOBAL
    virtual bool instantiated_from_constructor(void) const;
    virtual void print_type_name(std::ostream&) const;
    virtual void set_initialized_and_after_global_constructors(void) const;
#endif
  };

#ifdef DEBUGGLOBAL
  template<class TYPE, int inst, class CONVERTER = int>
  class GlobalTypeName { };

  template<class TYPE, int inst, class CONVERTER = int>
  class GlobalInstanceTypeName { };
#endif

} // namespace _internal_
} // namespace utils
/// @endcond

//==========================================================================================================
// Declarations

/**---------------------------------------------------------------------------------------------------------
 *
 * \class GlobalConverterVoid
 *
 * To be used as third template parameter of Global<class TYPE, int inst, class CONVERTER> when
 * the default constructor of TYPE must be called on instantiation.
 */

class GlobalConverterVoid
{
public:
  /// Return `void` to be used for the construction of `TYPE`.
  void operator()(int) { }
};

/**---------------------------------------------------------------------------------------------------------
 *
 * \class GlobalConverterInt
 *
 * The default third template parameter of Global<class TYPE, int inst, class CONVERTER>.
 * Using this class causes TYPE to be created by calling TYPE(int inst).
 */

class GlobalConverterInt
{
public:
  /// Return an `int` to be used for the construction of `TYPE`.
  int operator()(int inst) { return inst; }
};

//----------------------------------------------------------------------------------------------------------
//
// template<class TYPE, int inst, class CONVERTER>
// class Global

template<class TYPE, int inst, class CONVERTER = GlobalConverterInt>
class Global : public ::utils::_internal_::GlobalBase<TYPE, inst>
{
  typedef typename ::utils::_internal_::Instance<TYPE, inst, CONVERTER> Instance;
  typedef ::utils::_internal_::GlobalBase<TYPE, inst> base_type;
public:
  Global(void);
  ~Global();
  /// Returns a reference to the underlaying instance. Initialized the object first if necessary.
  static inline TYPE& instantiate(void);
#ifdef DOXYGEN
  // This is really part of utils::_internal_::GlobalBase.
  /// A reference to the underlaying instance.
  static inline TYPE& instance(void);
#endif
private:
  static void initialize_instance_(void);
};

#ifdef DEBUGGLOBAL
template<class TYPE, int inst, class CONVERTER>
std::ostream& operator<<(std::ostream& os, typename ::utils::_internal_::GlobalTypeName<TYPE, inst, CONVERTER> const&)
{
  char const* p = libcwd::type_info_of<Global<TYPE, inst, CONVERTER> >().demangled_name();
  size_t len = strlen(p);
  if (!strcmp(", int>", p + len - 6))
  {
    os.write(p, len - 6);
    os << "[, CONVERTER]>";
  }
  else if (len > 21 && !strcmp(", GlobalConverterInt>", p + len - 21))
  {
    os.write(p, len - 21);
    os << '>';
  }
  else
    os << p;
  return os;
}

template<class TYPE, int inst, class CONVERTER>
std::ostream& operator<<(std::ostream& os, typename ::utils::_internal_::GlobalInstanceTypeName<TYPE, inst, CONVERTER> const&)
{
  ::utils::_internal_::GlobalTypeName<TYPE, inst, CONVERTER> name;
  return operator<<(os, name);
}
#endif

#include "GlobalObjectManager.h"

//==========================================================================================================
// Definitions

template<class TYPE, int inst, class CONVERTER>
inline TYPE& Global<TYPE, inst, CONVERTER>::instantiate(void)
{
#ifdef DEBUGGLOBAL
  utils::_internal_::GlobalBase<TYPE, inst>::instantiate_function_name =
      libcwd::pc_mangled_function_name((char*)__builtin_return_address(0) + libcwd::builtin_return_address_offset);
  utils::_internal_::GlobalBase<TYPE, inst>::instantiate_return_address1 = __builtin_return_address(1);
#endif
  if (!base_type::initialized)
    initialize_instance_();
  return *static_cast<TYPE*>(reinterpret_cast<Instance*>(base_type::instance_));
}

template<class TYPE, int inst, class CONVERTER>
Global<TYPE, inst, CONVERTER>::Global(void)
{
#ifdef DEBUGGLOBAL
  if (utils::_internal_::GlobalBase<TYPE, inst>::instantiated_from_constructor)
  {
    typename utils::_internal_::GlobalInstanceTypeName<TYPE, inst, CONVERTER> name;
    __LibcwDoutFatal( dc::core,
	"The class `" << name << "' is defined more then once.\n"
	"          There should be one and only one code line reading:\n"
	"          static " << name << " dummy;" );
  }
  utils::_internal_::GlobalBase<TYPE, inst>::instantiated_from_constructor = true;
#endif
  if (!base_type::initialized)				// Only initialize when it wasn't initialized before
  {
    base_type::initialized = -1;			// Stop the next line from doing something if this is the GlobalObjectManager.
    Singleton<GlobalObjectManager>::instantiate();	// initialize_instance_() uses GlobalObjectManager
    initialize_instance_();
  }
#ifdef DEBUGGLOBAL
  else
    Singleton<GlobalObjectManager>::instantiate();	// We need to set instantiate_function_name/instantiate_return_address1 !
#endif
  Singleton<GlobalObjectManager>::instance().global_constructor_called();	// Update a counter
}

template<class TYPE, int inst, class CONVERTER>
Global<TYPE, inst, CONVERTER>::~Global()
{
  // Using instantiate() here instead of instance() to catch the case where someone wrote his own main().
  Singleton<GlobalObjectManager>::instantiate().global_destructor_called();
}

template<class TYPE, int inst, class CONVERTER>
void Global<TYPE, inst, CONVERTER>::initialize_instance_(void)
{
  base_type::initialized = -1;				// Stop Global<TYPE, inst>::Global() from calling us again.
  Instance* globalObject = new (base_type::instance_) Instance(inst);
  base_type::initialized = 1;
  Singleton<GlobalObjectManager>::instance().registerGlobalObject(globalObject);
}

namespace utils {
/// @cond Doxygen_Suppress
  namespace _internal_ {

    template<class TYPE, int inst, class CONVERTER>
    CONVERTER Instance<TYPE, inst, CONVERTER>::parameter_converter;

    template<class TYPE, int inst>
    char GlobalBase<TYPE, inst>::instance_[sizeof(InstanceDummy)] __attribute__((__aligned__));
    // remove this:
    // The sizeof(void*) is to add space
													// for virtual table pointer of
													// GlobalObject.

    template<class TYPE, int inst>
    char GlobalBase<TYPE, inst>::initialized;

    template<class TYPE, int inst>
    inline TYPE& GlobalBase<TYPE, inst>::instance(void)
    {
#ifdef DEBUGGLOBAL
      if (!initialized_and_after_global_constructors)
	check_call_to_instance();
#endif
      return *static_cast<TYPE*>(reinterpret_cast<InstanceDummy*>(instance_));	// If `instance_' is not yet initialized, then define DEBUGGLOBAL to find out why.
    }

#ifdef DEBUGGLOBAL
    template<class TYPE, int inst>
    bool GlobalBase<TYPE, inst>::initialized_and_after_global_constructors = false;

    template<class TYPE, int inst>
    char const* GlobalBase<TYPE, inst>::instantiate_function_name = NULL;

    template<class TYPE, int inst>
    void const* GlobalBase<TYPE, inst>::instantiate_return_address1 = NULL;

    template<class TYPE, int inst>
    bool GlobalBase<TYPE, inst>::instantiated_from_constructor = false;

    template<class TYPE, int inst>
    void GlobalBase<TYPE, inst>::print_error_msg(void)
    {
      //
      // You should use `instantiate()' instead of `instance()' in constructors of Singleton<> and Global<> objects.
      //
      // Note that when `instance()' is called from some function foobar() which is called from such a constructor,
      // add a call to `instantiate()' in that constructor *before* the call to foobar(), don't change `instance()'
      // into `instantiate()' inside foobar().
      //
      // If `instance()' was called after main(), then you forgot to add a global object Global<> or Singleton<>
      // to make sure that this object is instantiated before main() is called.
      //
      if (GlobalObjectManager::is_after_global_constructors())
      {
        GlobalInstanceTypeName<TYPE, inst> name;
	initialized = -2;		// Stop endless loop (instance() below calling print_err_msg() again).
	__LibcwDoutFatal( dc::core, "Missing global/static initialization of `" << name << "'.\n"
	    "          There should be one and only one code line reading:\n"
	    "          static " << name << " dummy;" );
      }
      else
	__LibcwDoutFatal( dc::core,
	    "Using `instance()' in global constructor.  Use `instantiate()' inside the\n"
	    "          constructor instead, or add `instantiate()' to the constructor before calling\n"
	    "          the function that calls `instance()' when `instance()' wasn't called directly\n"
	    "          by the constructor." );

    }

    template<class TYPE, int inst>
    void GlobalBase<TYPE, inst>::check_call_to_instance(void)
    {
      if (!initialized)
	print_error_msg();

      // Do a backtrace looking for a common function from which instantiate() was called:
      void* prev_addr = NULL;
      void* addresses[400];
      int depth = backtrace(addresses, sizeof(addresses) / sizeof(void*));
      int i = 0;
      while (i < depth)
      {
        void* addr = addresses[i];
	if (addr == instantiate_return_address1
	    && instantiate_function_name == libcwd::pc_mangled_function_name((char*)prev_addr + libcwd::builtin_return_address_offset))
	  break;
	prev_addr = addr;
	++i;
      }
      if (i == depth)
      {
	//
	// Calls to instantiate/instance should obey the following:
	//
	// libc function --> call to static/global constructor:		<-- instantiate_function_name
	//   ^                       {
	//   |                         call to instantiate()
// instantiate_return_address1         call to foobar1() --> call to foobar2() --> ... etc --> call to instance()
	//                           }
	//
        GlobalTypeName<TYPE, inst> name;
	libcwd::location_ct loc(inst < 0 ? ((char*)__builtin_return_address(2) + libcwd::builtin_return_address_offset)
	                                : ((char*)__builtin_return_address(1) + libcwd::builtin_return_address_offset));
        __LibcwDoutFatal(dc::core, loc << ": Calling " << name << "::instance() in (or indirectly from)\n"
            "          constructor of static or global object instead of (or without first) calling " << name << "::instantiate().");
      }
    }

    template<class TYPE, int inst, class CONVERTER>
    bool Instance<TYPE, inst, CONVERTER>::instantiated_from_constructor(void) const
    {
      return Global<TYPE, inst, CONVERTER>::gifc_();
    }

    template<class TYPE, int inst>
    bool Instance<TYPE, inst, GlobalConverterVoid>::instantiated_from_constructor(void) const
    {
      return Global<TYPE, inst, GlobalConverterVoid>::gifc_();
    }

    template<class TYPE, int inst, class CONVERTER>
    void Instance<TYPE, inst, CONVERTER>::print_type_name(std::ostream& os) const
    {
      typename utils::_internal_::GlobalInstanceTypeName<TYPE, inst, CONVERTER> name;
      os << name;
    }

    template<class TYPE, int inst>
    void Instance<TYPE, inst, GlobalConverterVoid>::print_type_name(std::ostream& os) const
    {
      typename utils::_internal_::GlobalInstanceTypeName<TYPE, inst, GlobalConverterVoid> name;
      os << name;
    }

    template<class TYPE, int inst, class CONVERTER>
    void Instance<TYPE, inst, CONVERTER>::set_initialized_and_after_global_constructors(void) const
    {
      Global<TYPE, inst, CONVERTER>::set_initialized_and_after_global_constructors();
    }

    template<class TYPE, int inst>
    void Instance<TYPE, inst, GlobalConverterVoid>::set_initialized_and_after_global_constructors(void) const
    {
      Global<TYPE, inst, GlobalConverterVoid>::set_initialized_and_after_global_constructors();
    }

#endif // DEBUGGLOBAL

  }	// namespace _internal_
/// @endcond
}	// namespace libcw

#endif // UTILS_GLOBAL_H

/*
// NOTE1

This note gives a diagnose for the problem:

YourProgram.cc: `YourClass::YourClass()' is private
utils/Global.h:107: within this context

Of course it is possible that `YourClass' is NOT a singleton and you just made its constructor
private by accident.  However, if `YourClass' is a singleton then its constructor should
indeed be private and the following should help to fix the problem:

The following compiler error (using egcs-2.95.1):

utils/Global.h: In method `utils::_internal_::Instance<Bar,-1,GlobalConverterVoid>::Instance(int)':
utils/Global.h:357:   instantiated from `Global<Bar,-1,GlobalConverterVoid>::initialize_instance_()'
utils/Global.h:342:   instantiated from `Global<Bar,-1,GlobalConverterVoid>::Global()'
Singleton_tst.cc:114:   instantiated from here
Singleton_tst.cc:77: `Bar::Bar()' is private
utils/Global.h:107: within this context

Means that you forgot to add a `friend_Instance' at the top of your singleton class `Bar'.

If instead you get this error:

utils/Global.h: In method `utils::_internal_::Instance<Bar,0,GlobalConverterVoid>::Instance(int)':
utils/Global.h:357:   instantiated from `Global<Bar,0,GlobalConverterVoid>::initialize_instance_()'
utils/Global.h:342:   instantiated from `Global<Bar,0,GlobalConverterVoid>::Global()'
Singleton_tst.cc:110:   instantiated from here
Singleton_tst.cc:77: `Bar::Bar()' is private
utils/Global.h:107: within this context

Where the `0' can be any positive integer.
Then instead you seem to be trying to instantiate `static Global<Bar, 0, GlobalConverterVoid>'.
Instead, use `static Singleton<Bar>' for singletons.

// NOTE2

This note gives a diagnose for the problem:

YourProgram.cc: `YourClass::YourClass()' is private
utils/Global.h:88: within this context

Of course it is possible that `YourClass' is NOT a singleton and you just made its constructor
private by accident.  However, if `YourClass' is a singleton then its constructor should
indeed be private and the following should help to fix the problem:

The following compiler error (using egcs-2.95.1):

utils/Global.h: In method `utils::_internal_::Instance<Bar,-1,YourGlobalConverter>::Instance(int)':
utils/Global.h:357:   instantiated from `Global<Bar,-1,YourGlobalConverter>::initialize_instance_()'
utils/Global.h:342:   instantiated from `Global<Bar,-1,YourGlobalConverter>::Global()'
Singleton_tst.cc:108:   instantiated from here
Singleton_tst.cc:90: `Bar::Bar(SomeType)' is private
utils/Global.h:94: within this context

Means you are trying to instantiate `Global<Bar,-1,YourGlobalConverter>' in an
attempt to pass a parameter to the singleton `Bar'.  This is however not possible.
A singleton can ONLY have a default constructor. You will have to use another
Global<> object to initialize a singleton (let the singleton read initialization
data from that other Global<> object).

*/
