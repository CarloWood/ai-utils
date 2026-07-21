// SPDX-FileCopyrightText: 2014-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaraction of class GlobalObjectManager.
 */

#include "Global.h"		// Must be included before GlobalObjectManager.h

#ifndef UTILS_GLOBAL_OBJECT_MANAGER_H
#define UTILS_GLOBAL_OBJECT_MANAGER_H

#include "Singleton.h"
#include "debug.h"

#include <vector>

#if defined(CWDEBUG) && CWDEBUG_ALLOC
#include <libcwd/private_internal_stringbuf.h>
#endif

/// Global objects manager singleton class.
///
/// This singleton is used by Global<> to keep track of the number
/// of global instances and their destruction.
///
class GlobalObjectManager : public Singleton<GlobalObjectManager>
{
  friend_Instance;
private:
#if defined(CWDEBUG) && CWDEBUG_ALLOC
  using globalObjects_type = std::vector<utils::_internal_::GlobalObject*,
      libcwd::_private_::auto_internal_allocator::rebind<utils::_internal_::GlobalObject*>::other>;
#else
  using globalObjects_type = std::vector<utils::_internal_::GlobalObject*>;
#endif
  globalObjects_type globalObjects;
  int number_of_global_objects;

private:
  ~GlobalObjectManager() { }
  GlobalObjectManager(GlobalObjectManager const&) : Singleton<GlobalObjectManager>() { }	// Never used

private:
  template<class TYPE, int inst, class CONVERTER> friend class Global;
  void global_constructor_called() { ++number_of_global_objects; }
  template<class TYPE, int inst> friend class utils::_internal_::GlobalBase;
  void global_destructor_called() {
#ifdef DEBUGGLOBAL
    if (!after_global_constructors)
      DoutFatal(dc::core, "When you link with libcw, you should not define your own main(). "
                         "Perhaps you intended to only link with libcwd?");
#endif
    if (--number_of_global_objects == 0)
      deleteGlobalObjects();
  }
  void deleteGlobalObjects();

public:
  void registerGlobalObject(utils::_internal_::GlobalObject* globalObject);

#ifndef DEBUGGLOBAL
private:
  GlobalObjectManager() : number_of_global_objects(0) { }
#else
private:
  static bool after_global_constructors;
  GlobalObjectManager() : number_of_global_objects(0) { }
public:
  static void main_entered();	// Should be called at the top of main() - sets `after_global_constructors' and does checking.
  static bool is_after_global_constructors() { return after_global_constructors; }
#endif
};

#endif // UTILS_GLOBAL_OBJECT_MANAGER_H
