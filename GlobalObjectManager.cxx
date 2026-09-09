// SPDX-FileCopyrightText: 2014-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of class GlobalObjectManager.
 */

#include "sys.h"
#include "GlobalObjectManager.h"
#include "debug.h"
#ifdef CWDEBUG
#include "print_using.h"
#endif

using namespace utils::_internal_;

#ifdef DEBUGGLOBAL
bool GlobalObjectManager::after_global_constructors = false;
#endif

/// @cond Doxygen_Suppress
void GlobalObjectManager::registerGlobalObject(GlobalObject* globalObject)
{
  globalObjects.push_back(globalObject);
}

void GlobalObjectManager::deleteGlobalObjects()
{
  bool done;
  do
  {
    GlobalObject* globalObject = globalObjects.back();
    globalObjects.pop_back();
    done = globalObjects.empty();
    if (!done)				// Don't call the destructor of GlobalObjectManager itself! (last one is self)
      globalObject->~GlobalObject();
  }
  while(!done);
}
/// @endcond

#ifdef DEBUGGLOBAL
void GlobalObjectManager::main_entered()
{
  Singleton<GlobalObjectManager>::instantiate();
  for (globalObjects_type::const_iterator i(Singleton<GlobalObjectManager>::instance().globalObjects.begin());
       i != Singleton<GlobalObjectManager>::instance().globalObjects.end();
       ++i)
  {
    if (!(*i)->instantiated_from_constructor())
    {
      DoutFatal( dc::core,
	  "Missing global/static initialization of `" << utils::print_using(*(*i), &GlobalObject::print_type_name) << "'.\n"
	  "          There should be one and only one code line reading:\n"
	  "          static " << utils::print_using(*(*i), &GlobalObject::print_type_name) << " dummy;" );
    }
    (*i)->set_initialized_and_after_global_constructors();
  }
  Singleton<GlobalObjectManager>::instance().after_global_constructors = true;
}
#endif

namespace {

SingletonInstance<GlobalObjectManager> GlobalObjectManager_instance __attribute__ ((unused));

} // namespace
